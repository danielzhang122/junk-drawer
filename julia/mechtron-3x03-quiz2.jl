#quiz 2
#I used claude ai to help me understand the process and start a framework/skeleton for to generate a symmetric matrix which i later completed
#I also used claude to help debug the histogram an plotting the histogram
import LinearAlgebra as linalg
import Random as rand
import Plots as plots

rand.seed!(2025) #RNG seed 2025

function genSymmtric(size::Int) #function that generates symmetric random matrix and returns eigenvalues
    A = rand.randn(size, size)
    symmetric = linalg.Symmetric(A)
    return linalg.eigvals(symmetric)
end

# print eigenvalue with highest and lowest magnitude for size 7
eig = genSymmtric(7)
max = argmax(abs.(eig))
min = argmin(abs.(eig))
println("highest eigenvalue ", eig[max])
println("lowest eigenvaluie ", eig[min])

# generate N=10 matrices of size 128x128 and plot histogram
N = 10
size = 128
totalEigenvalues = Float64[]

for i in 1:N
    eigenvals = genSymmtric(size)
    append!(totalEigenvalues, eigenvals)
end

plot =plots.histogram(totalEigenvalues, bins=30, title="Histogram of Eigenvalues (Size 128, N=10)", xlabel="Eigenvalue", ylabel="Frequency")
display(plot)
# is it following a trend?
println("Question: is it following a trend?: i think so lol")