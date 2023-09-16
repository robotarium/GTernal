clear all
close all
clc

cd ../Data/MotorVelocity
load motorTransferFunction
load validationDataRandomSpeed

%% Model Created Earlier
modelOutput = lsim(dsysCL, input, linspace(0,10,0.05));

%% Plot Data
figure(1)
hold on
plot(time./1000,input,'g','LineWidth',2)
plot(time(1:end-1)./1000,motor1Vel,'LineWidth',2);
plot(time(1:end-1)./1000,motor2Vel,'r','LineWidth',2);
plot(time./1000,modelOutput,'k','LineWidth',3);
hold off
grid on
box on
%axis([0 10 -10 10])
set(gca,'fontweight','bold','fontsize',14)
xlabel('Time (s)','fontweight','bold','fontsize',16)
ylabel('Shaft Speed (rad/s)','fontweight','bold','fontsize',16)
title('Random Speed 100 Gear Ratio','fontweight','bold','fontsize',20)

legend('Desired Velocity','Motor 1','Motor 2','Model')