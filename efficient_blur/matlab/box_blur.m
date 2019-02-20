% Tesbench for box-blur on images to compare C++ against MATLAB

clc,  close all;

% TODO: Get this value from C++
N = 512;

% % Synthetic image
% x = 1:N*N;
% x = reshape(x,[N,N])'
x = double(rgb2gray(imread('lena_512.png')));

h = ones(3,3);
z_gold = conv2(x,h,'same');

figure, 
subplot(2,1,1), imshow(z_gold,[]), title('golden reference');
error = sprintf('L2-norm: %2.2f',norm(abs(z_gold-cpp),2))
subplot(2,1,2), imshow(cpp, []), title(error);