clc, clear, close all;

N = 8;

x = 1:N*N;

x = reshape(x,[N,N])'
h = [1,1,1];

y = zeros(N,N); % 4+3-1
for i = 1:N
    y(:,i) = conv(x(i,:),h,'same')';
end
% Transposing implicitly

% y = zeros(4,6);
for i = 1:N
    z(:,i) = conv(y(i,:),h,'same')';
end
z_seperable = z % Transposing implicitly

h = ones(3,3);
z_gold = conv2(x,h,'same')