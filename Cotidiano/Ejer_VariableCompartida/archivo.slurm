#!/bin/bash
#SBATCH --job-name=sumaCompartida
#SBATCH --output=resultado_variable.txt
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=5
#SBATCH --mem=1G
#SBATCH --time=00:05:00
#SBATCH --partition=kura

./variableCompartida
