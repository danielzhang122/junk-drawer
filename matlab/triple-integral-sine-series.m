clear; clc;

integrand = @(t, r, w) ((r.*sin(t) + 6).^2 + (w + 8).^2) .* r;

Ix = integral3(integrand, ...
               0, 2*pi, ...
               0, sqrt(2), ...
               @(t,r) r, ...
               @(t,r) sqrt(4 - r.^2));

fprintf('%.6f\n', Ix);

clear; clc; close all;

f = @(x) sqrt(1000 + x.^3);
L = 10;

x_pos = linspace(0, 10, 201);
x_plot = linspace(-20,20,1000);
N_terms = 7;

f_odd = @(x) (x>=0).*f(x) + (x<0).*(-f(-x));
b = zeros(1,N_terms);
for n = 1:N_terms
    integrand = f(x_pos).*sin(n*pi*x_pos/L);
    b(n) = (2/L)*trapz(x_pos, integrand);
end
sine_series = @(x, N) arrayfun(@(x0) sum( b(1:N) .* sin((1:N)*pi*x0/L) ), x);

figure;
hold on;
plot(x_pos, f(x_pos), 'r-', 'LineWidth', 3);
x_neg = linspace(-10,0,100);
plot(x_neg, f_odd(x_neg), 'b-', 'LineWidth', 3);
for n = 1:N_terms
    series_vals = sine_series(x_plot, n);
    plot(x_plot, series_vals, 'k-', 'LineWidth', 1);
end
xlim([-20,20]);
grid on;
hold off;
saveas(gcf, 'part_a_odd_extension.png');

f_even = @(x) (x>=0).*f(x) + (x<0).*f(-x);
a = zeros(1, N_terms+1);
a(1) = (2/L)*trapz(x_pos, f(x_pos));
for n = 1:N_terms
    integrand = f(x_pos).*cos(n*pi*x_pos/L);
    a(n+1) = (2/L)*trapz(x_pos, integrand);
end
cosine_series = @(x, N) a(1)/2 + arrayfun(@(x0) sum( a(2:N+1) .* cos((1:N)*pi*x0/L) ), x);

figure;
hold on;
plot(x_pos, f(x_pos), 'r-', 'LineWidth', 3);
plot(x_neg, f_even(x_neg), 'b-', 'LineWidth', 3);
for n = 1:N_terms
    series_vals = cosine_series(x_plot, n);
    plot(x_plot, series_vals, 'k-', 'LineWidth', 1);
end
xlim([-20,20]);
grid on;
hold off;
saveas(gcf, 'part_b_even_extension.png');

f_id = @(x) (x>=0).*f(x) + (x<0).*f(x+10);
x_full = linspace(-10,10, 2*201-1);
f_full = f_id(x_full);
a_full = zeros(1, N_terms+1);
b_full = zeros(1, N_terms+1);
a_full(1) = (1/L)*trapz(x_full, f_full);
for n = 1:N_terms
    a_full(n+1) = (1/L)*trapz(x_full, f_full.*cos(n*pi*x_full/L));
    b_full(n+1) = (1/L)*trapz(x_full, f_full.*sin(n*pi*x_full/L));
end
fourier_series = @(x, N) a_full(1)/2 + arrayfun(@(x0) sum( a_full(2:N+1).*cos((1:N)*pi*x0/L) + b_full(2:N+1).*sin((1:N)*pi*x0/L) ), x);

figure;
hold on;
plot(x_pos, f(x_pos), 'r-', 'LineWidth', 3);
plot(x_neg, f_id(x_neg), 'b-', 'LineWidth', 3);
for n = 1:N_terms
    series_vals = fourier_series(x_plot, n);
    plot(x_plot, series_vals, 'k-', 'LineWidth', 1);
end
xlim([-20,20]);
grid on;
hold off;
saveas(gcf, 'part_c_identity_reflection.png');

x = linspace(0, 10, 201);
f = sqrt(1000 + x.^3);
actual_value = sqrt(1000 + 5^3);

bn = zeros(1, N_terms);
for n = 1:N_terms
    integrand = f .* sin(n * pi * x / 10);
    bn(n) = 0.2 * trapz(x, integrand);
end
approx_a = sum(bn .* sin((1:N_terms) * pi * 5 / 10));
error_a = abs(actual_value - approx_a);

a0 = 0.2 * trapz(x, f);
an = zeros(1, N_terms);
for n = 1:N_terms
    integrand = f .* cos(n * pi * x / 10);
    an(n) = 0.2 * trapz(x, integrand);
end
approx_b = a0/2 + sum(an .* cos((1:N_terms) * pi * 5 / 10));
error_b = abs(actual_value - approx_b);

a0_c = (1/5) * trapz(x, f);
an_c = zeros(1, N_terms);
for n = 1:N_terms
    integrand = f .* cos(n * pi * x / 5);
    an_c(n) = (1/5) * trapz(x, integrand);
end
approx_c = a0_c/2 + sum(an_c .* (-1).^(1:N_terms));
error_c = abs(actual_value - approx_c);

disp([error_a, error_b, error_c]);
