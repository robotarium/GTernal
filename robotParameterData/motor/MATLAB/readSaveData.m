clear all
close all
clc

cd ../Data/MotorVelocity/

%% Record Data From Serial Port
i = 1;
inputFrequency = [0.01, 0.03, 0.05, 0.1, 0.3, 0.5, 1, 3, 5, 10, 20, 25, 50];
inputStepMagnitude = [5:10:255];
motorGearRatio = 150.58;
encoderCountsPerRotation = 12; %% Encoder counts per shaft rotation.
data=[];

s = serial('/dev/ttyACM0','BaudRate', 115200);
fopen(s);
s.ReadAsyncMode = 'continuous';
readasync(s);

while (1)
    strData = fscanf(s);
    splitData = strsplit(strData,',');
    if (length(splitData) == 1)
        break
    end
    data(i,:)=[str2double(splitData(1)) str2double(splitData(2)) str2double(splitData(3)) str2double(splitData(4))];
    i=i+1;
end
fclose(s);
clear s splitData strData i

%% Data Management
time = data(:,3);
motor1 = 2*pi*data(:,1)./(motorGearRatio * encoderCountsPerRotation); % In Radians
motor2 = 2*pi*data(:,2)./(motorGearRatio * encoderCountsPerRotation); % In Radians
input = data(:,4);

%Fix Negative Values
j = 1;
for i = 2:length(time) %Find 32bit rollover index.
    if time(i)<0 && time(i-1)>0
        index(j)=i;
        j=j+1;
    end
end

if (exist('index','var') > 0)
    for k = 1:length(index)
        time(index(k):end) = time(index(k):end) + (2^32 -1); %This deals with the 32 bit rollover.
    end
end

%Create Sampling Times
samplingTime = time(2:end)-time(1:end-1);
disp('Sampling Time Average');
disp(mean(samplingTime));

motor1Vel = (motor1(2:end) - motor1(1:end-1))./(samplingTime) * 1000;
motor2Vel = (motor2(2:end) - motor2(1:end-1))./(samplingTime) * 1000;

%% Save Data
sFile = 'rawFittingData.csv';
csvwrite(sFile,data(2:end,:),1,0);   % save the data to a CSV file
save('fittingData','data','inputFrequency','inputStepMagnitude','time','motor1','motor2','input','samplingTime','motor1Vel','motor2Vel','motorGearRatio','encoderCountsPerRotation');

disp('DONE!')