data = load('math2zz3_hw1_data_file.txt');
P = data(:, 1);
Q = data(:, 2);
R = data(:, 3);
t = data(:, 4);
rx = 7 * P;
ry = 8 * Q;
rz = 2 * R;

figure;
plot3(rx, ry, rz, 'LineWidth', 2);
title('Daniel Zhang');
xlabel('7P(t)');
ylabel('8Q(t)');
zlabel('2R(t)');
grid on;
saveas(gcf, '3D_Curve_Plot.png');

rx = 9 * P;
ry = 3 * Q;
rz = 9 * R;
N = length(t);
Tx = zeros(N, 1);
Ty = zeros(N, 1);
Tz = zeros(N, 1);
Nx = zeros(N, 1);
Ny = zeros(N, 1);
Nz = zeros(N, 1);
Bx = zeros(N, 1);
By = zeros(N, 1);
Bz = zeros(N, 1);

for i = 1:N-1
    drx = rx(i+1) - rx(i);
    dry = ry(i+1) - ry(i);
    drz = rz(i+1) - rz(i);
    magT = sqrt(drx^2 + dry^2 + drz^2);
    magT = max(magT, eps);
    Tx(i) = drx / magT;
    Ty(i) = dry / magT;
    Tz(i) = drz / magT;
end
Tx(N) = Tx(N-1);
Ty(N) = Ty(N-1);
Tz(N) = Tz(N-1);

for i = 1:N-1
    dTx = Tx(i+1) - Tx(i);
    dTy = Ty(i+1) - Ty(i);
    dTz = Tz(i+1) - Tz(i);
    magN = sqrt(dTx^2 + dTy^2 + dTz^2);
    magN = max(magN, eps);
    Nx(i) = dTx / magN;
    Ny(i) = dTy / magN;
    Nz(i) = dTz / magN;
end
Nx(N) = Nx(N-1);
Ny(N) = Ny(N-1);
Nz(N) = Nz(N-1);

for i = 1:N
    B = cross([Tx(i), Ty(i), Tz(i)], [Nx(i), Ny(i), Nz(i)]);
    magB = sqrt(B(1)^2 + B(2)^2 + B(3)^2);
    magB = max(magB, eps);
    Bx(i) = B(1) / magB;
    By(i) = B(2) / magB;
    Bz(i) = B(3) / magB;
end

figure;
plot3(rx, ry, rz, 'k', 'LineWidth', 2);
hold on;
indices = 20:200:N;
scaleFactor = 5.0;
for i = indices
    quiver3(rx(i), ry(i), rz(i), Tx(i)*scaleFactor, Ty(i)*scaleFactor, Tz(i)*scaleFactor, 'r', 'LineWidth', 1.5, 'MaxHeadSize', 2);
    quiver3(rx(i), ry(i), rz(i), Nx(i)*scaleFactor, Ny(i)*scaleFactor, Nz(i)*scaleFactor, 'g', 'LineWidth', 1.5, 'MaxHeadSize', 2);
    quiver3(rx(i), ry(i), rz(i), Bx(i)*scaleFactor, By(i)*scaleFactor, Bz(i)*scaleFactor, 'b', 'LineWidth', 1.5, 'MaxHeadSize', 2);
    plot3(rx(i), ry(i), rz(i), 'ko', 'MarkerSize', 6, 'MarkerFaceColor', 'y');
end
title('Daniel Zhang', 'FontSize', 14);
xlabel('9P(t)', 'FontSize', 12);
ylabel('3Q(t)', 'FontSize', 12);
zlabel('9R(t)', 'FontSize', 12);
grid on;
legend('Curve', 'Tangent (T)', 'Normal (N)', 'Binormal (B)', 'Markers', 'Location', 'best');
view(3);
saveas(gcf, 'Enhanced_3D_Curve_With_Vectors.png');
