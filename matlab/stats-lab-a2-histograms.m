%% Lab A2: Full Statistical Analysis with Histograms

%% Part A: Nuts Analysis (Distance Across Flats)
nut_names = {'Nut A', 'Nut B', 'Nut C'};
nut_data = {
    [18.86, 18.86, 18.86, 18.85, 18.87, 18.86, 18.85, 18.86, 18.86], % Nut A
    [18.85, 18.86, 18.85, 18.85, 18.84, 18.84, 18.85, 18.85, 18.84], % Nut B
    [18.84, 18.87, 18.84, 18.84, 18.85, 18.86, 18.84, 18.86, 18.85]  % Nut C
};

fprintf('===== PART A: NUTS =====\n');
for i = 1:length(nut_names)
    data = nut_data{i};
    
    % Calculations
    mean_val = mean(data);
    std_dev = std(data);
    SEM = std_dev / sqrt(length(data));
    conf_interval = 2.306 * SEM; % t-value for ν=8 (95% CI)
    min_val = min(data);
    max_val = max(data);
    bin_width = (max_val - min_val) / 5;
    
    % Print statistics
    fprintf('\n=== %s ===\n', nut_names{i});
    fprintf('Min: %.4f mm | Max: %.4f mm | Bin Width: %.4f mm\n', min_val, max_val, bin_width);
    fprintf('Mean: %.4f mm\n', mean_val);
    fprintf('Standard Deviation: %.4f mm\n', std_dev);
    fprintf('95%% Confidence Interval: [%.4f, %.4f] mm\n', ...
        mean_val - conf_interval, mean_val + conf_interval);
    
    % Plot histogram
    figure;
    histogram(data, 'BinWidth', bin_width, 'FaceColor', [0.2 0.6 0.8], 'EdgeColor', 'k');
    hold on;
    xline(mean_val, 'r-', 'LineWidth', 2, 'Label', 'Mean');
    xline([mean_val - std_dev, mean_val + std_dev], 'g--', 'LineWidth', 1.5, 'Label', '±1 Std Dev');
    xline([mean_val - conf_interval, mean_val + conf_interval], 'b-.', 'LineWidth', 1.5, 'Label', '95%% CI');
    ylabel('Frequency');
    xlabel('Distance Across Flats (mm)');
    title(sprintf('%s Histogram', nut_names{i}));
    legend('Histogram', 'Mean', 'Std Dev', '95%% CI', 'Location', 'northwest');
    grid on;
    hold off;
end

%% Part B: Bars Analysis (Thickness) - Histogram by Location
bar_names = {'Bar A', 'Bar B', 'Bar C'};
locations = {'Edge 1', 'Middle', 'Edge 2'};
bar_data = {
    % Edge 1          Middle           Edge 2
    [6.432, 6.441, 6.432; 6.372, 6.383, 6.382; 6.441, 6.405, 6.405], % Bar A
    [6.415, 6.448, 6.414; 6.374, 6.382, 6.432; 6.432, 6.444, 6.414], % Bar B
    [6.423, 6.442, 6.424; 6.373, 6.375, 6.473; 6.414, 6.400, 6.415]  % Bar C
};

fprintf('\n\n===== PART B: BARS =====\n');
for j = 1:length(locations)
    % Extract data for each location across all bars
    data = [bar_data{1}(j,:), bar_data{2}(j,:), bar_data{3}(j,:)]; 

    % Calculations
    mean_val = mean(data);
    std_dev = std(data);
    SEM = std_dev / sqrt(length(data));
    conf_interval = 2.306 * SEM; % t-value for ν=8 (95% CI)
    min_val = min(data);
    max_val = max(data);
    bin_width = (max_val - min_val) / 5;

    % Print statistics
    fprintf('\n=== %s ===\n', locations{j});
    fprintf('Min: %.4f mm | Max: %.4f mm | Bin Width: %.4f mm\n', min_val, max_val, bin_width);
    fprintf('Mean: %.4f mm\n', mean_val);
    fprintf('Standard Deviation: %.4f mm\n', std_dev);
    fprintf('95%% Confidence Interval: [%.4f, %.4f] mm\n', ...
        mean_val - conf_interval, mean_val + conf_interval);

    % Plot histogram
    figure;
    histogram(data, 'BinWidth', bin_width, 'FaceColor', [0.8 0.4 0.2], 'EdgeColor', 'k');
    hold on;
    xline(mean_val, 'r-', 'LineWidth', 2, 'Label', 'Mean');
    xline([mean_val - std_dev, mean_val + std_dev], 'g--', 'LineWidth', 1.5, 'Label', '±1 Std Dev');
    xline([mean_val - conf_interval, mean_val + conf_interval], 'b-.', 'LineWidth', 1.5, 'Label', '95%% CI');
    ylabel('Frequency');
    xlabel('Thickness (mm)');
    title(sprintf('%s Thickness Histogram', locations{j}));
    legend('Histogram', 'Mean', 'Std Dev', '95%% CI', 'Location', 'northwest');
    grid on;
    hold off;
end
