clear; clc
figure(1); clf

a=csvRead('log.csv');
plot(a(:,1),a(:,2),'r');
plot(a(:,1),a(:,3),'k');
