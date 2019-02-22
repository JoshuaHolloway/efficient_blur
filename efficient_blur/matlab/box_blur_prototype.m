close all, clc;

x = 1:N*N;

x = reshape(x,[N,N])'
h = [1,1,1];

y = zeros(N,N);
for i = 1:N
    y(:,i) = conv(x(i,:),h,'same')';
end
% Transposing implicitly

z = zeros(N,N);
for i = 1:N
    z(:,i) = conv(y(i,:),h,'same')';
end
z_seperable = z % Transposing implicitly

h = ones(3,3) / 9;
z_gold = conv2(x,h,'same')

figure, 
subplot(2,1,1), imshow(z_gold,[]), title('golden reference');
error = sprintf('L2-norm: %2.2f',norm(abs(z_gold - z_cpp),2))
subplot(2,1,2), imshow(z_cpp, []), title(error);