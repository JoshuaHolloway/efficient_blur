close all, clc;

N = 6;

x = 1:N*N;

x = reshape(x,[N,N])'
x = padarray(x, [1 1])

h = [1,1,1] / 3;

% y = zeros(N,N);
for i = 1:N
    temp = conv(x(i,:),h)
    y(i,:) = temp(3:6);
%     y(:,i) = conv(x(i,:),h,'same')';
end
y

% y = padarray(y,[1, 1])
% Transposing implicitly

% z = zeros(N,N);
% for i = 1:N
%     z(:,i) = conv(y(i,:),h,'same')';
% end
% z_seperable = z % Transposing implicitly
% 
% h = ones(3,3);
% z_gold = conv2(x,h,'same')
% 
