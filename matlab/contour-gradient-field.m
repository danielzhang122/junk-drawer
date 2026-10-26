clear; clc;

x = linspace(-1,1,100);
y = linspace(-1,1,100);
[X, Y] = meshgrid(x,y);

u = 3*(1-3*X).^2.*exp(-X.^2 - (3*Y+1).^2) ...
    - 10*(3*X/5 - 27*X.^3 - 243*Y.^5).*exp(-5*X.^2-4*Y.^2) ...
    - 1/3*exp(-(3*X+1).^2 - 9*Y.^2) + (X.^2+Y.^2) - 1;

figure;
contourf(X, Y, u, 20);
colorbar;
hold on;

xq = linspace(-1,1,20);
yq = linspace(-1,1,20);
[Xq, Yq] = meshgrid(xq, yq);

uq = 3*(1-3*Xq).^2.*exp(-Xq.^2 - (3*Yq+1).^2) ...
    - 10*(3*Xq/5 - 27*Xq.^3 - 243*Yq.^5).*exp(-5*Xq.^2-4*Yq.^2) ...
    - 1/3*exp(-(3*Xq+1).^2 - 9*Yq.^2) + (Xq.^2+Yq.^2) - 1;

[ux, uy] = gradient(uq, xq, yq);

quiver(Xq, Yq, ux, uy, 'k', 'LineWidth', 1);

title('Daniel Zhang');


hold off;

clear; clc;

u = @(x, y) 3*(1-3*x).^2.*exp(-x.^2 - (3*y+1).^2) ...
    - 10*(3*x/5 - 27*x.^3 - 243*y.^5).*exp(-5*x.^2-4*y.^2) ...
    - 1/3*exp(-(3*x+1).^2-9*y.^2) + (x.^2+y.^2) - 1;

gradFun = @(r) [...
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

r_final = r(end, :);
x_final = r_final(1);
y_final = r_final(2);

z_final = u(x_final, y_final);


fprintf(' (%.5f, %.5f)\n', x_final, y_final);
fprintf(' %.5f\n', z_final);

