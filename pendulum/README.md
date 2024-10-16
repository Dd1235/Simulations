# Double Pendulum Simulation (Work in Progress)

This repo contains my attempt at simulating the double pendulum.
We use Lagrangian mechanics to get second order ODE's for the angles of the pendulum.
Euler method is the simplest, but not very accurate. It hasn't been implemented here.
I have used the RK4 method but it ends up failing for large time steps.

## To do

- Symplectic integrator, that is, an integrator that conserves energy at each time step.
- Variational integrator, an integrator that makes sure the solution satisfies the Euler-Lagrange equations. (Tries to get d/dt(dL/dqdot) - dL/dq = 0)
- Implement Rk78 bench mark integrator
- Look into using AI for the problem.
