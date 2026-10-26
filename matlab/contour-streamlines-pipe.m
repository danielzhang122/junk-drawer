clear; clc;

x = linspace(-1, 1, 100);
y = linspace(-1, 1, 100);
[X, Y] = meshgrid(x, y);

u = @(x, y) 3*(1-3*x).^2.*exp(-x.^2 - (3*y+1).^2) ...
    - 10*(3*x/5 - 27*x.^3 - 243*y.^5).*exp(-5*x.^2-4*y.^2) ...
    - 1/3*exp(-(3*x+1).^2-9*y.^2) + (x.^2+y.^2) - 1;

U = u(X, Y);

figure;
contourf(X, Y, U, 20);
colorbar;
hold on;

gradFun = @(r) [ ...
    -( -(18*(1-3*r(1)))*exp(-r(1)^2 - (3*r(2)+1)^2) ...
       - 6*(1-3*r(1))^2*r(1)*exp(-r(1)^2 - (3*r(2)+1)^2) ...
       - (10*(3/5-81*r(1)^2))*exp(-5*r(1)^2-4*r(2)^2) ...
       + (20*((3/5)*r(1)-27*r(1)^3-243*r(2)^5))*5*r(1)*exp(-5*r(1)^2-4*r(2)^2) ...
       - (1/3*(-18*r(1)-6))*exp(-(3*r(1)+1)^2-9*r(2)^2) + 2*r(1) ); ...
    -( 3*(1-3*r(1))^2*(-18*r(2)-6)*exp(-r(1)^2-(3*r(2)+1)^2) ...
       + 12150*r(2)^4*exp(-5*r(1)^2-4*r(2)^2) ...
       + (20*((3/5)*r(1)-27*r(1)^3-243*r(2)^5))*4*r(2)*exp(-5*r(1)^2-4*r(2)^2) ...
       + 6*r(2)*exp(-(3*r(1)+1)^2-9*r(2)^2) + 2*r(2) ) ];

odeFun = @(t, r) gradFun(r);

r0 = [-0.14; 0.7];
tspan = 0:0.001:5.0;

[t, r] = ode45(odeFun, tspan, r0);

x_stream = r(:, 1);
y_stream = r(:, 2);

u_stream = u(x_stream, y_stream);

sign_stream = sign(u_stream);
sign_stream(sign_stream == 0) = 1;

N = length(t);
seg_start = 1;
for i = 2:N
    if sign_stream(i) ~= sign_stream(i-1)
        if sign_stream(i-1) > 0
            color_seg = 'w';
        else
            color_seg = 'k';
        end
        plot(x_stream(seg_start:i-1), y_stream(seg_start:i-1), 'Color', color_seg, 'LineWidth', 2);
        seg_start = i;
    end
end
if sign_stream(end) > 0
    color_seg = 'w';
else
    color_seg = 'k';
end
plot(x_stream(seg_start:end), y_stream(seg_start:end), 'Color', color_seg, 'LineWidth', 2);

title('Daniel Zhang');


hold off;

x_stream = r(:,1);
y_stream = r(:,2);
z_stream = u(x_stream, y_stream);

dx = diff(x_stream);
dy = diff(y_stream);
dz = diff(z_stream);

ds = sqrt(dx.^2 + dy.^2 + dz.^2);

L = sum(ds);

fprintf(' %.5f \n', L);

x_source = r(1, 1);
y_source = r(1, 2);
z_source = u(x_source, y_source);

x_end = r(end, 1);
y_end = r(end, 2);
z_end = u(x_end, y_end);

pipe_length = sqrt((x_end - x_source)^2 + (y_end - y_source)^2 + (z_end - z_source)^2);

fprintf('%.5f \n', pipe_length);

x_source = r(1, 1);
y_source = r(1, 2);
z_source = u(x_source, y_source);

x_end = r(end, 1);
y_end = r(end, 2);
z_end = u(x_end, y_end);

height_diff = abs(z_end - z_source);

fprintf('%.5f\n', height_diff);