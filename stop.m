speeds = [1, 4, 8, 9, 10, 11, 12, 13];
velocities = [57, 1700, 3772, 4237, 4769, 5231, 5664, 5740];
stops = [[0, 0, 0],
         [199000, 199000, 199000],
         [460000, 462000, 437000],
         [510000, 510000, 505000],
         [585000, 583000, 575000],
         [642000, 650000, 665000],
         [726000, 725000, 711000],
         [783000, 791000, 792000]];
average_stops = mean(stops, 2);
stop_function = polyfit(velocities, average_stops', 1);


velocities_50 = [0, 660, 1253, 1753, 2213, 2733, 3286, 3827, 4340, 4827, 5286];
stops_50 = [0, 5.6, 13.4, 19.4, 26.4, 32.5, 41.6, 45.5, 50.9, 56.2, 65];
stops_50_um = 10000 * stops_50;
stops_50_function = polyfit(velocities_50, stops_50_um, 1);

x = 0 : 1 : 5286;
y = polyval(stops_50_function, x);

%deceleration_function = polyder(stop_function);
%x2 = 0 : 5800;
%y2 = polyval(deceleration_function, x2);

hold on;
title('Speed/Velocity vs Stopping Distance')
%plot(speeds, average_stops, '*')
plot(x,y,'-');
%poly(x',y','-');
plot(velocities_50, stops_50_um, 'x');
hold off;
