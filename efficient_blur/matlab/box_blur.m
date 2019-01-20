clc, clear, close all;
x = 1:16;

x = reshape(x,[4,4])'
h = [1,1,1];

y = zeros(4,6);
for i = 1:4
    y(i,:) = conv(x(i,:),h);
end
y
