ticks = [0, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200, 220, 240, 260, 280, 300, 320, 340, 360, 380, 400, 420];
distances = [0, 0, 9, 30, 53, 87, 105, 135, 195, 238, 287, 383, 462, 556, 665, 860, 960, 1180, 1310, 1385, 1470, 1565];
distances_um = distances * 1000;
acceleration_poly = polyfit(ticks, distances_um, 4);
xs = 0 : 450;
ys = polyval(acceleration_poly, xs);

bpoly = polyder(acceleration_poly);
bys = polyval(bpoly, xs);

hold on;
plot(ticks, distances_um, '*');
plot(xs, ys);
%plot(xs, bys);
hold off;