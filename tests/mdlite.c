/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

double dist(int nd, double r1[], double r2[], double dr[]);

#ifdef USE_xtask
#include <xtask.h>
double PI2 = 3.141592653589793 / 2.0;
xtask_config xc;

typedef struct {
    xtask_task task;
    int nd;
    double *pos;
    int k, j;
    double *f, pe; // Output
} taskdata;

typedef struct {
    xtask_task task;
    int np, nd;
    double *pos, *vel;
    int k;
    double *f, pe, ke; // Outputs
    double *subf;
    taskdata *subtask;
} mdlite_task;

void *thirdloop(void* dummy, void* vdata) {
    mdlite_task* data = vdata;
    /*
          Compute the kinetic energy.
     */
    data->pe = 0;
    data->ke = 0;
    for(int i=0; i < data->nd; i++) {
        data->f[i+data->k*data->nd] = 0;
        data->ke += pow(data->vel[i+data->k*data->nd], 2);
    }

    for(int j = 0; j < data->np; j++) {
        data->pe += data->subtask[j].pe;
        for(int i=0; i < data->nd; i++)
		data->f[i+data->k*data->nd] += data->subtask[j].f[i];
    }

    free(data->subf);

    return NULL;
}

void *secondloop(void* dummy, void* vdata) {
    taskdata* data = vdata;
    if (data->k != data->j) {
	double rij[data->nd];

        double d = dist(data->nd,
		data->pos + (data->k * data->nd),
		data->pos + (data->j * data->nd),
		rij);
        /*
          Attribute half of the potential energy to particle J.
         */
        double d2;
        if (d < PI2) {
            d2 = d;
        } else {
            d2 = PI2;
        }

        data->pe = 0.5 * pow(sin(d2), 2);

        for (int i = 0; i < data->nd; i++) {
            data->f[i] = - rij[i] * sin(2.0 * d2) / d;
        }
    } else {
        data->pe = 0;
        for (int i = 0; i < data->nd; i++) data->f[i] = 0;
    }

    return NULL;
}

void *firstloop(void* dummy, void* vdata) {
    mdlite_task* data = vdata;

    data->subf = malloc(data->np*data->nd*sizeof(double));
    data->task = (xtask_task){thirdloop, XTASK_FATE_LEAF, &data->subtask[0], NULL};
    for (int j = 0; j < data->np; j++) {
	data->subtask[j] = (taskdata){
            {secondloop, XTASK_FATE_LEAF, NULL, &data->subtask[j+1]},
            data->nd, data->pos, data->k, j, &data->subf[j*data->nd],
        };
    }
    data->subtask[data->np-1].task.sibling = NULL;

    return data;
}
#endif

int main(int argc, char *argv[]);
void compute(int np, int nd, double pos[], double vel[],
        double mass, double f[], double *pot, double *kin);
double cpu_time();
void initialize(int np, int nd, double pos[], double vel[], double acc[]);
void r8mat_uniform_ab(int m, int n, double a, double b, int *seed, double r[]);
void timestamp();
void update(int np, int nd, double pos[], double vel[], double f[],
        double acc[], double mass, double dt);


/******************************************************************************/

int main(int argc, char *argv[]) {
    double *acc;
    double ctime;
    double dt;
    double e0;
    double *force;
    int i;
    int id;
    double kinetic;
    double mass = 1.0;
    int nd;
    int np;
    double *pos;
    double potential;
    int step;
    int step_num;
    int step_print;
    int step_print_index;
    int step_print_num;
    double *vel;

    timestamp();
    printf("\n");
    printf("MD\n");
    printf("  C version\n");
    printf("  A molecular dynamics program.\n");
    /*
      Get the spatial dimension.
     */
    if (1 < argc) {
        nd = atoi(argv[1]);
    } else {
        printf("\n");
        printf("  Enter ND, the spatial dimension (2 or 3).\n");
        scanf("%d", &nd);
    }
    //
    //  Get the number of particles.
    //
    if (2 < argc) {
        np = atoi(argv[2]);
    } else {
        printf("\n");
        printf("  Enter NP, the number of particles (500, for instance).\n");
        scanf("%d", &np);
    }
    //
    //  Get the number of time steps.
    //
    if (3 < argc) {
        step_num = atoi(argv[3]);
    } else {
        printf("\n");
        printf("  Enter ND, the number of time steps (500 or 1000, for instance).\n");
        scanf("%d", &step_num);
    }
    //
    //  Get the time steps.
    //
    if (4 < argc) {
        dt = atof(argv[4]);
    } else {
        printf("\n");
        printf("  Enter DT, the size of the time step (0.1, for instance).\n");
        scanf("%lg", &dt);
    }

#ifdef USE_xtask
    xc = (xtask_config){.workers = 1};
    if (5 < argc) {
        xc.workers = atoi(argv[5]);
    }
#endif

    /*
      Report.
     */
    printf("\n");
    printf("  ND, the spatial dimension, is %d\n", nd);
    printf("  NP, the number of particles in the simulation, is %d\n", np);
    printf("  STEP_NUM, the number of time steps, is %d\n", step_num);
    printf("  DT, the size of each time step, is %f\n", dt);
#ifdef USE_xtask
    printf("  W, the number of workers, is %d\n", xc.workers);
#endif
    /*
      Allocate memory.
     */
    acc = (double *) malloc(nd * np * sizeof ( double));
    force = (double *) malloc(nd * np * sizeof ( double));
    pos = (double *) malloc(nd * np * sizeof ( double));
    vel = (double *) malloc(nd * np * sizeof ( double));
    /*
      This is the main time stepping loop:
        Compute forces and energies,
        Update positions, velocities, accelerations.
     */
    printf("\n");
    printf("  At each step, we report the potential and kinetic energies.\n");
    printf("  The sum of these energies should be a constant.\n");
    printf("  As an accuracy check, we also print the relative error\n");
    printf("  in the total energy.\n");
    printf("\n");
    printf("      Step      Potential       Kinetic        (P+K-E0)/E0\n");
    printf("                Energy P        Energy K       Relative Energy Error\n");
    printf("\n");

    step_print = 0;
    step_print_index = 0;
    step_print_num = 10;

    ctime = cpu_time();

    for (step = 0; step <= step_num; step++) {
        if (step == 0) {
            initialize(np, nd, pos, vel, acc);
        } else {
            update(np, nd, pos, vel, force, acc, mass, dt);
        }

        compute(np, nd, pos, vel, mass, force, &potential, &kinetic);

        if (step == 0) {
            e0 = potential + kinetic;
        }

        if (step == step_print) {
            printf("  %8d  %14f  %14f  %14e\n", step, potential, kinetic,
                    (potential + kinetic - e0) / e0);
            step_print_index = step_print_index + 1;
            step_print = (step_print_index * step_num) / step_print_num;
        }

    }
    /*
      Report timing.
     */
    ctime = cpu_time() - ctime;
    printf("\n");
    printf("  Elapsed cpu time: %f seconds.\n", ctime);
    /*
      Free memory.
     */
    free(acc);
    free(force);
    free(pos);
    free(vel);
    /*
      Terminate.
     */
    printf("\n");
    printf("MD\n");
    printf("  Normal end of execution.\n");
    printf("\n");
    timestamp();

    return 0;
}

/******************************************************************************/

void compute(int np, int nd, double pos[], double vel[], double mass,
        double f[], double *pot, double *kin) {
    double d;
    double d2;
    int i;
    int j;
    int k;
    double ke;
    double pe;
    double PI2 = 3.141592653589793 / 2.0;
    double rij[3];

    pe = 0.0;
    ke = 0.0;

#ifdef USE_xtask
    mdlite_task* mdtask = malloc(np*sizeof(mdlite_task));
    taskdata* subs = malloc(np*np*sizeof(taskdata));
    for (k = 0; k < np; k++) {
        mdtask[k] = (mdlite_task){
            {firstloop, 0, NULL, &mdtask[k+1]}, np, nd, pos, vel, k,
            f, 0, 0, NULL, &subs[k*np]};
    }
    mdtask[np-1].task.sibling = NULL;
    xtask_run(mdtask, xc);

    for(k = 0; k < np; k++) {
        pe += mdtask[k].pe;
        ke += mdtask[k].ke;
    }
    free(subs);
    free(mdtask);
#else
    for (k = 0; k < np; k++) {
        /*
          Compute the potential energy and forces.
         */
        for (i = 0; i < nd; i++) {
            f[i + k * nd] = 0.0;
        }

        for (j = 0; j < np; j++) {
            if (k != j) {
                d = dist(nd, pos + k*nd, pos + j*nd, rij);
                /*
                  Attribute half of the potential energy to particle J.
                 */
                if (d < PI2) {
                    d2 = d;
                } else {
                    d2 = PI2;
                }

                pe = pe + 0.5 * pow(sin(d2), 2);

                for (i = 0; i < nd; i++) {
                    f[i + k * nd] = f[i + k * nd] - rij[i] * sin(2.0 * d2) / d;
                }
            }
        }
        /*
          Compute the kinetic energy.
         */
        for (i = 0; i < nd; i++) {
            ke = ke + vel[i + k * nd] * vel[i + k * nd];
        }
    }
#endif

    ke = ke * 0.5 * mass;

    *pot = pe;
    *kin = ke;

    return;
}

/*******************************************************************************/

double cpu_time() {
    double value;

    value = (double) clock() / (double) CLOCKS_PER_SEC;

    return value;
}

/******************************************************************************/

double dist(int nd, double r1[], double r2[], double dr[]) {
    double d;
    int i;

    d = 0.0;
    for (i = 0; i < nd; i++) {
        dr[i] = r1[i] - r2[i];
        d = d + dr[i] * dr[i];
    }
    d = sqrt(d);

    return d;
}

/******************************************************************************/

void initialize(int np, int nd, double pos[], double vel[], double acc[]) {
    int i;
    int j;
    int seed;
    /*
      Set positions.
     */
    seed = 123456789;
    r8mat_uniform_ab(nd, np, 0.0, 10.0, &seed, pos);
    /*
      Set velocities.
     */
    for (j = 0; j < np; j++) {
        for (i = 0; i < nd; i++) {
            vel[i + j * nd] = 0.0;
        }
    }
    /*
      Set accelerations.
     */
    for (j = 0; j < np; j++) {
        for (i = 0; i < nd; i++) {
            acc[i + j * nd] = 0.0;
        }
    }

    return;
}

/******************************************************************************/

void r8mat_uniform_ab(int m, int n, double a, double b, int *seed, double r[]) {
    int i;
    const int i4_huge = 2147483647;
    int j;
    int k;

    if (*seed == 0) {
        fprintf(stderr, "\n");
        fprintf(stderr, "R8MAT_UNIFORM_AB - Fatal error!\n");
        fprintf(stderr, "  Input value of SEED = 0.\n");
        exit(1);
    }

    for (j = 0; j < n; j++) {
        for (i = 0; i < m; i++) {
            k = *seed / 127773;

            *seed = 16807 * (*seed - k * 127773) - k * 2836;

            if (*seed < 0) {
                *seed = *seed + i4_huge;
            }
            r[i + j * m] = a + (b - a) * (double) (*seed) * 4.656612875E-10;
        }
    }

    return;
}

/******************************************************************************/

void timestamp() {
#define TIME_SIZE 40

    static char time_buffer[TIME_SIZE];
    const struct tm *tm;
    size_t len;
    time_t now;

    now = time(NULL);
    tm = localtime(&now);

    len = strftime(time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm);

    printf("%s\n", time_buffer);

    return;
#undef TIME_SIZE
}

/******************************************************************************/

void update(int np, int nd, double pos[], double vel[], double f[],
        double acc[], double mass, double dt) {
    int i;
    int j;
    double rmass;

    rmass = 1.0 / mass;

    for (j = 0; j < np; j++) {
        for (i = 0; i < nd; i++) {
            pos[i + j * nd] = pos[i + j * nd] + vel[i + j * nd] * dt + 0.5 * acc[i + j * nd] * dt * dt;
            vel[i + j * nd] = vel[i + j * nd] + 0.5 * dt * (f[i + j * nd] * rmass + acc[i + j * nd]);
            acc[i + j * nd] = f[i + j * nd] * rmass;
        }
    }

    return;
}
