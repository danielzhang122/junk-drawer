data = load('math2zz3_hw1_data_file.txt'); 
P = data(:, 1); 
Q = data(:, 2); 
R = data(:, 3); 
r = [9*P, 3*Q, 9*R]; 
numPoints = size(r, 1); 
T = zeros(numPoints-1, 3); 
N = zeros(numPoints-1, 3); 
B = zeros(numPoints-2, 3); 

for i = 1:numPoints-1
    T(i, :) = r(i+1, :) - r(i, :);
    T(i, :) = T(i, :) / norm(T(i, :)); 
end

for i = 1:numPoints-2
    N(i, :) = T(i+1, :) - T(i, :);
    N(i, :) = N(i, :) / norm(N(i, :)); 
end

for i = 1:numPoints-2
    B(i, :) = cross(T(i, :), N(i, :));
    B(i, :) = B(i, :) / norm(B(i, :)); 
end

r_prime = diff(r); 
r_double_prime = diff(r_prime); 
curvature = zeros(numPoints - 2, 1);

for i = 1:numPoints-2
    numerator = norm(cross(r_prime(i, :), r_double_prime(i, :)));
    denominator = norm(r_prime(i, :))^3;
    curvature(i) = numerator / denominator;
end

max_curvature = max(curvature);
min_curvature = min(curvature);
curvature_difference = max_curvature - min_curvature;

disp('Maximum curvature:');
disp(max_curvature);
disp('Minimum curvature:');
disp(min_curvature);
disp('Difference between max and min curvature:');
disp(curvature_difference);

Tdx = diff(T(:,1));  
Tdy = diff(T(:,2));  
Tdz = diff(T(:,3));  

norm_rprime = sqrt(r_prime(1:end-1,1).^2 + r_prime(1:end-1,2).^2 + r_prime(1:end-1,3).^2);
norm_Tprime = sqrt(Tdx.^2 + Tdy.^2 + Tdz.^2); 

curvature_Tprime = norm_Tprime ./ norm_rprime;

max_curvature_Tprime = max(curvature_Tprime);
min_curvature_Tprime = min(curvature_Tprime);

curvature_difference_Tprime = max_curvature_Tprime - min_curvature_Tprime;

disp('Maximum curvature based on T''(t):');
disp(max_curvature_Tprime);
disp('Minimum curvature based on T''(t):');
disp(min_curvature_Tprime);
disp('Difference between max and min curvature based on T''(t):');
disp(curvature_difference_Tprime);
