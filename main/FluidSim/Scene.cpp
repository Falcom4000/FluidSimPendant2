#include "Scene.h"
#include "st7789.h"
extern "C" void Scene::init(Vec center)
{
    fluid = (uint8_t*)heap_caps_calloc(1, blockSize, MALLOC_CAP_DMA);
    background = (uint8_t*)heap_caps_calloc(1, blockSize, MALLOC_CAP_DMA);
    memset(background, 0xff, blockSize);
    memset(grid, 0, sizeof(grid)); // Reset grid
    memset(isFluid, 0 ,sizeof(isFluid));
    memset(isFluid_old, 0 ,sizeof(isFluid_old));
    memset(fluid, 0xff, blockSize);
    // for (int i = blockGap; i < displayScale - blockGap; i++) {
    //     for (int j = blockGap; j < displayScale - blockGap; j++) {
    //         fluid[i * displayScale * BytePerPixel + j * BytePerPixel] = 0;
    //         fluid[i * displayScale * BytePerPixel + j * BytePerPixel + 1] = 0;
    //     }
    // }
    for(int i = 0; i < blockSize/BytePerPixel;){
        background[i] = (WHITE >> 8) & 0xFF;
        fluid[i++] = (BLUE >> 8) & 0xFF;
        background[i] = WHITE & 0xFF;
		fluid[i++] = BLUE & 0xFF;
    }
    srand(static_cast<unsigned int>(time(nullptr)));
    particles.clear();
    add_object(center, 1000);
}

extern "C" void IRAM_ATTR Scene::update(real dt, Vector3 G)
{
    memset(grid, 0, sizeof(grid)); // Reset grid
    for (auto& p : particles) { // P2G
        Vec base_coord = Vec(floor(p.x.x * inv_dx - 0.5f), floor(p.x.y * inv_dx - 0.5f));
        Vec fx = p.x * inv_dx - base_coord;
        // Quadratic kernels  [http://mpm.graphics   Eqn. 123, with x=fx, fx-1,fx-2]
        Vec w[3] { Vec(0.5) * sqr(Vec(1.5) - fx), Vec(0.75) - sqr(fx - Vec(1.0)),
            Vec(0.5) * sqr(fx - Vec(0.5)) };
        Mat r, s;
        Mathutils::polar_decomp(p.F, r, s); // Polar decomp. for fixed corotated model
        Mat cauchy = Mat(0.2F * E * (p.Jp - 1));
        auto stress = cauchy * -4 * inv_dx * inv_dx * dt * vol;
        auto affine = stress + p.C * particle_mass;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) { // Scatter to grid
                auto dpos = (Vec(i, j) - fx) * dx;
                Vector3 mv((p.v * particle_mass).x, (p.v * particle_mass).y,
                    particle_mass); // translational momentum
                grid[int(base_coord.x + i)][int(base_coord.y + j)] += (mv + Vector3((affine * dpos).x, (affine * dpos).y, 0)) * w[i].x * w[j].y;
            }
        }
    }
    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= n; j++) { // For all grid nodes
            auto& g = grid[i][j];
            if (g.z > 0) { // No need for epsilon here
                g /= g.z; //        Normalize by mass
                g += G * dt * GravityScale; //                  Gravity
                real x = (real)i / n;
                real y = (real)j / n; // boundary thick.,node coord
                if (x < boundary || x > 1 - boundary || y > 1 - boundary)
                g = Vector3(0); // Sticky
                if (y < boundary)
                    g.y = std::max(0.0F, g.x); //"Separate"

            }
        }
    }
    for (auto& p : particles) { // Grid to particle
        Vec base_coord = Vec(floor(p.x.x * inv_dx - 0.5f), floor(p.x.y * inv_dx - 0.5f));
        Vec fx = p.x * inv_dx - base_coord;
        Vec w[3] { Vec(0.5) * sqr(Vec(1.5) - fx), Vec(0.75) - sqr(fx - Vec(1.0)),
            Vec(0.5) * sqr(fx - Vec(0.5)) };
        p.C = Mat(0);
        p.v = Vec(0);
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                auto dpos = (Vec(i, j) - fx);
                auto grid_ij = grid[int(base_coord.x + i)][int(base_coord.y + j)];
                auto grid_v = Vec(grid_ij.x, grid_ij.y);
                auto weight = w[i].x * w[j].y;
                p.v += grid_v * weight; // Velocity
                p.C += Mat::outer_product(grid_v * weight, dpos) * 4 * inv_dx; // APIC C
            }
        }
        p.x += p.v * dt; // Advection
        p.Jp *= (Mat(1) + p.C * dt).determinant();
    }
}

extern "C" void Scene::add_object(Vec center, int AddNum)
{
    for (int i = 0; i < AddNum; i++) {
        real dx = real(rand()) / RAND_MAX;
        real dy = real(rand()) / RAND_MAX;
        particles.emplace_back(center + Vec((dx - 0.5) * 0.5, (dy - 0.5) * 0.20));
    }
    std::sort(particles.begin(), particles.end(), [](const Particle& a, const Particle& b) {
        return a.x.y < b.x.y;
    });
}
extern "C" IRAM_ATTR void Scene::render(TFT_t* dev)
{
    for (auto& p : particles) { // Grid to particle
        Vec base_coord = Vec(floor(p.x.x * inv_dx - 0.5f), floor(p.x.y * inv_dx - 0.5f));
        int i = int(base_coord.x);
        int j = int(base_coord.y);
        isFluid[i][j] = 1;
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if ((isFluid[i][j] ^ isFluid_old[i][j]) == 1) {
                if (isFluid[i][j] == 1) {
                    lcdDrawFillRect2(dev, i * displayScale, j * displayScale, (i + 1) * displayScale, (j + 1) * displayScale, BLUE);
                }
                else {
                    lcdDrawFillRect2(dev, i * displayScale, j * displayScale, (i + 1) * displayScale, (j + 1) * displayScale, WHITE);
                }
            }
        }
    }
    memcpy(isFluid_old, isFluid, sizeof(isFluid));
    memset(isFluid, 0, sizeof(isFluid));
}