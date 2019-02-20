clc, clear, close all;
x = 1:16;

x = reshape(x,[4,4])'
h = [1,1,1];

y = zeros(4,4);
for i = 1:4
    y(:,i) = conv(x(i,:),h,'same')';
end
% Transposing implicitly

% y = zeros(4,6);
for i = 1:4
    z(:,i) = conv(y(i,:),h,'same')';
end
z_seperable = z % Transposing implicitly

h = ones(3,3);
z_gold = conv2(x,h,'same')