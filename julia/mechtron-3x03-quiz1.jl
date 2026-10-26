#=mechtron 3x03 quiz 1 Daniel Zhang

=#

import LinearAlgebra as linalg
x64 = [2.1 3.3 3.4 3.3; 3.3 0.5 3.8 0.9; 3.2 3.8 3.7 3.5; 3.3 0.9 1.4 0.2] #standard is Float64

function f(x) #f(x)=diag(cos(x))
    cosMatrix = cos.(x)
    return linalg.diag(cosMatrix)
end

x32=Float32.(x64) #convert to Float32


diagcos64 = f(x64) #diagonalize and cos when x is 64 bit
diagcos32= f(x32) # ^ ^ but with 32

max64=maximum(diagcos64) #find maximum for 64 and 32
max32=maximum(diagcos32)

absDiff= abs(max64-max32) #difference between maximums for float32 and 64

machineEps32 = eps(max32::Float32) #machine epsilone for float32 and 64
machineEps64 = eps(max64::Float64)

println("absolute difference between 32 and 64 bit floats: ", absDiff)


println(absDiff/machineEps32 ) #ratio of the difference to machine epsilon
println(absDiff/machineEps64) #ratio of the difference to machine epsilon