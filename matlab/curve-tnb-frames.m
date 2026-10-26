% Load the dataset 
data = load('math2zz3_hw1_data_file.txt'); 

% Extract P(t), Q(t), R(t)
P = data(:, 1); % First column
Q = data(:, 2); % Second column
R = data(:, 3); % Third column

% Define r(t) = [9P(t), 3Q(t), 9R(t)]
r = [9*P, 3*Q, 9*R];

% Initialize arrays for Tangent T(t), Normal N(t), and Binormal B(t)
numPoints = size(r, 1);
T = zeros(numPoints-1, 3); % Tangents
N = zeros(numPoints-1, 3); % Normals
B = zeros(numPoints-2, 3); % Binormals (B is 1 less since it uses N and T)

% Compute tangent vectors T(t) using forward difference
for i = 1:numPoints-1
    T(i, :) = r(i+1, :) - r(i, :);
    T(i, :) = T(i, :) / norm(T(i, :)); % Normalize tangent
end

% Compute normal vectors N(t)
for i = 1:numPoints-2
    N(i, :) = T(i+1, :) - T(i, :);
    N(i, :) = N(i, :) / norm(N(i, :)); % Normalize normal
end

% Compute binormal vectors B(t) using the cross product
for i = 1:numPoints-2
    B(i, :) = cross(T(i, :), N(i, :));
    B(i, :) = B(i, :) / norm(B(i, :)); % Normalize binormal
end

% Compute first derivative r'(t) using forward difference
r_prime = diff(r); % Forward difference for r'(t)

% Compute second derivative r''(t) using forward difference
r_double_prime = diff(r_prime); % Forward difference for r''(t)

% Preallocate curvature array
curvature = zeros(numPoints - 2, 1);

% Compute curvature at each point
for i = 1:numPoints-2
    numerator = norm(cross(r_prime(i, :), r_double_prime(i, :)));
    denominator = norm(r_prime(i, :))^3;
    curvature(i) = numerator / denominator;
end

% Find maximum and minimum curvature
max_curvature = max(curvature);
min_curvature = min(curvature);

% Compute the difference
curvature_difference = max_curvature - min_curvature;

% Display curvature results
disp('Maximum curvature:');
disp(max_curvature);
disp('Minimum curvature:');
disp(min_curvature);
disp('Difference between max and min curvature:');
disp(curvature_difference);

% --------------------------
% Additional Curvature Calculation based on T'(t)
% --------------------------

% Derivatives for T(t)
Tdx = diff(T(:,1));  % T_x derivative
Tdy = diff(T(:,2));  % T_y derivative
Tdz = diff(T(:,3));  % T_z derivative

% Norms of r'(t) and T'(t)
norm_rprime = sqrt(r_prime(1:end-1,1).^2 + r_prime(1:end-1,2).^2 + r_prime(1:end-1,3).^2);
norm_Tprime = sqrt(Tdx.^2 + Tdy.^2 + Tdz.^2); 

% Calculate curvature from T'(t)
curvature_Tprime = norm_Tprime ./ norm_rprime;

% Find the max and min curvature from T'(t)
max_curvature_Tprime = max(curvature_Tprime);
min_curvature_Tprime = min(curvature_Tprime);

% Calculate the curvature difference from T'(t)
curvature_difference_Tprime = max_curvature_Tprime - min_curvature_Tprime;

% Display additional curvature results
disp('Maximum curvature based on T''(t):');
disp(max_curvature_Tprime);
disp('Minimum curvature based on T''(t):');
disp(min_curvature_Tprime);
disp('Difference between max and min curvature based on T''(t):');
disp(curvature_difference_Tprime);
