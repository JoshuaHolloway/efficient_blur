clc, close all;

% Testbench to test the C++ prototype

x = 1:M*N;
x = reshape(x,[N,M])'
h = ones(K,K) / (K^2);
z_gold = conv2(x,h,'same')

figure, 
subplot(2,1,1), imshow(z_gold,[]), title(sprintf(...
    'golden reference $ %d \\times %d $', M, N), 'Interpreter', 'latex');
error = sprintf(...
    'C++\nError (L2-norm of difference): %2.2f',norm(z_gold - z_cpp,2));
subplot(2,1,2), imshow(z_cpp, []), title(error);

if (z_gold ~= z_cpp)
    error('C++ implementation is not correct');
end