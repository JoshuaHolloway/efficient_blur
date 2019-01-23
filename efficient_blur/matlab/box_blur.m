clc, clear, close all;
x = 1:16;

x = reshape(x,[4,4])'
h = [1,1,1];

y = zeros(6,6); % 4+3-1
for i = 1:4
    y(i,:) = conv(x(i,:),h);
end
y
y_ = y'

% y = zeros(4,6);
for i = 1:4
    z(i,:) = conv(y_(i,:),h);
end
z
z_ = z'

h = ones(3,3);
z_gold = conv2(x,h,'same')