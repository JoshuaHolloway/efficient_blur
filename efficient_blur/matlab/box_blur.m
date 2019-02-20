% Tesbench for box-blur on images to compare C++ against MATLAB

clc,  close all;

% TODO: Get this value from C++
N = 512;

% % Synthetic image
% x = 1:N*N;
% x = reshape(x,[N,N])'
x = double(rgb2gray(imread('lena_512.png')));

h = ones(3,3) ./ 9;
z_gold = conv2(x,h,'same');

figure, 
subplot(3,1,1), imshow(z_gold,[]), title('golden reference');

error_z1 = sprintf('C++ Imp-1\nL2-norm: %2.2f',norm(abs(z_gold-z1_cpp),2))
subplot(3,1,2), imshow(z1_cpp, []), title(error_z1);

error_z4 = sprintf('C++ Imp-4\nL2-norm: %2.2f',norm(abs(z_gold-z4_cpp),2))
subplot(3,1,3), imshow(z4_cpp, []), title(error_z4);

figure
mesh(abs(z_gold-z1_cpp))