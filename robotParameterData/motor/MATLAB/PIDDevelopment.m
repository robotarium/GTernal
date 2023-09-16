%% Tune the PID for Pheeno Motor Control.
clear all
close all
clc

%% Now from Arduino Number (PWM D/A) to Velocity!

% Continuous Time ModelsID second order pole is on the order of 10^7)
K=1.264;
a=43.86;
s = tf('s');
motorP = K/(s+a);

%PID gains tuned for <1s settling time, <4% overshoot.
Kp = 6.01;
Ki = 713;
Kd = 0;

C = pid(Kp,Ki,Kd);
sysCL = feedback(C*motorP,1);

t = 0:0.01:1;
figure(4)
step(sysCL,t)
grid on
title('Step Response with PID Control')

% Discrete Time Model

Ts = 0.005;%Sampling Time
motorPD = c2d(motorP,Ts,'zoh');%Discrete Model

dsysCL = feedback(motorPD,1);
[y,t] = step(dsysCL,1);
figure(5)
stairs(t,y);
grid on
xlabel('Time (s)')
ylabel('Velocity (rad/s)')
title('Stairstep Response: Original')

%Test Continuous Time PID controller!
dC = c2d(Ki*(1+0.044*s)/(s*(1+0.012*s)),Ts);
dsysCL = feedback(dC*motorPD,1);
[y2,t] = step(dsysCL,5);
figure(6)
stairs(t,y2)
grid on
xlabel('Time (seconds)')
ylabel('Velocity (rad/s)')

%It works but there is a lot of overshoot and the settling time is long! 
%Not what we designed for. Lets fix that!

figure(7)
rlocus(dC*motorPD)
axis([-1.5 1.5 -1 1])
title('Root Locus of Compensated System')

%Test Continuous Time PID controller!
dsysCL = feedback(dC*motorPD,1);
[y3,t] = step(dsysCL,5);
figure(8)
stairs(t,y3)
grid on
title('Stairstep Response: with PID controller')
xlabel('Time (seconds)')
ylabel('Velocity (rad/s)')

