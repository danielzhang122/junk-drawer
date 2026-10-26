% Number of intervals and step size
N = 100;
dx = 1/N;

% Create x values using left endpoints from 0 to 1-dx
x = 0:dx:(1-dx);

% Path A: Straight line y = x
yA = x;
F_A = -11 * (yA + 0.8).^2.4;
Work_A = sum(F_A) * dx;

% Path B: Curved path y = x^9
yB = x.^9;
F_B = -11 * (yB + 0.8).^2.4;
Work_B = sum(F_B) * dx;

% Output results
fprintf('%.4f\n', Work_A);
fprintf('%.4f\n', Work_B);

% Higher precision setup
N = 1000;
x = linspace(0, 1, N+1);
dx = x(2)-x(1);

% Path A
yA = x;
dyA = diff(yA);
FxA = -11 * (yA(1:end-1) + 0.8).^2.4;
FyA = -26.4 * x(1:end-1) .* (yA(1:end-1) + 0.8).^1.4;
Work_A = sum(FxA * dx + FyA .* dyA);

% Path B
yB = x.^9;
dyB = diff(yB);
FxB = -11 * (yB(1:end-1) + 0.8).^2.4;
FyB = -26.4 * x(1:end-1) .* (yB(1:end-1) + 0.8).^1.4;
Work_B = sum(FxB * dx + FyB .* dyB);

% Output refined results
fprintf('%.4f\n', Work_A);
fprintf('%.4f\n', Work_B);

% Exact work using potential function
exactWork = -11*(1.8)^2.4;
fprintf('%.4f\n', exactWork);

% Compute work done on each segment
W1 = integral(@(t) sin(t.^2), 0, 8);
term1 = 39*log(39) - 39;
term2 = 7*log(7) - 7;
W2 = (term1 - term2) / 8;
W3 = integral(@(s) -8*sin(80*s.^2) - 4*log(7 + 32*s.^2), 0, 1);

% Total work
total_work = W1 + W2 + W3;
fprintf('%.4f\n', total_work);
