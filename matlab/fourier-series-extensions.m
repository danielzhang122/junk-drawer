% Define the common parameters
x = linspace(0, 10, 201); % 200 subintervals
f = sqrt(1000 + x.^3);
actual_value = sqrt(1000 + 5^3); % Actual f(5)

% Part (a): Odd extension (Fourier sine series)
N_terms = 7;
bn = zeros(1, N_terms);
for n = 1:N_terms
    integrand = f .* sin(n * pi * x / 10);
    bn(n) = 0.2 * trapz(x, integrand);
end
approx_a = sum(bn .* sin((1:N_terms) * pi * 5 / 10)); % Evaluate at x=5
error_a = abs(actual_value - approx_a);

% Part (b): Even extension (Fourier cosine series)
a0 = 0.2 * trapz(x, f);
an = zeros(1, N_terms);
for n = 1:N_terms
    integrand = f .* cos(n * pi * x / 10);
    an(n) = 0.2 * trapz(x, integrand);
end
approx_b = a0/2 + sum(an .* cos((1:N_terms) * pi * 5 / 10)); % Evaluate at x=5
error_b = abs(actual_value - approx_b);

% Part (c): Identity reflection (periodic extension with period 10)
a0_c = (1/5) * trapz(x, f);
an_c = zeros(1, N_terms);
for n = 1:N_terms
    integrand = f .* cos(n * pi * x / 5);
    an_c(n) = (1/5) * trapz(x, integrand);
end
approx_c = a0_c/2 + sum(an_c .* (-1).^(1:N_terms)); % Evaluate at x=5
error_c = abs(actual_value - approx_c);

% Display the errors
errors = [error_a, error_b, error_c];
disp(errors);