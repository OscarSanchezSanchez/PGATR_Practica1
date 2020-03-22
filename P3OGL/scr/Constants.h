#pragma once
#define NUM_PARTICLES 512*512//1204*1024
#define WORK_GROUP_SIZE 128

#define PARTICLE_LIFETIME 20000
#define LIFETIME_MAX 10000

//Disco
#define XMIN -1
#define XMAX 0.5 * 3.1415
#define YMIN 0
#define YMAX 0.2
#define ZMIN 0
#define ZMAX 2

#define VXMIN 0
#define VXMAX 2
#define VYMIN 0
#define VYMAX 1
#define VZMIN 0
#define VZMAX 1

//Cubo
#define XMAX1 0.1 * 3.1415
#define RADIO 1
#define VXMAX1 0.5
#define ATRAC_INCR 0.1

#define BITONIC false
