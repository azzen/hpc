import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import glob
import os

input_images = glob.glob("../img/*.png")
test_matrix = []
num_clusters = 5

for i, image in enumerate(input_images):
    test_matrix.append(f"\"{image} {num_clusters} out.png\"")

command = f"""
hyperfine --runs 50 \
            -L param {",".join(test_matrix)} \
            --export-csv benchmark_segmentation.csv \
            '../code/build/segmentation {{param}}'
"""

print(command)

if not os.path.exists("benchmark_segmentation.csv"):
    os.system(command)
    if os.path.exists("out.png"):
        print("Cleaning up...")
        os.system("rm out.png")
else:
    print("Benchmark already exists skipping...")

print("Generating graphs")

df = pd.read_csv("benchmark_segmentation.csv")

# Strip prefix from parameters column
df["parameter_param"] = df["parameter_param"].str.replace("../img/", "")
df["parameter_param"] = df["parameter_param"].str.replace(f"{num_clusters} out.png", "")

# rename parametesr_param to parameters
df = df.rename(columns={"parameter_param": "parameters"})

# convert s to ms
df["mean"] = df["mean"] * 1000

print(df)
# plot rect graph for each image with two bars for each type
fig, ax = plt.subplots()
width = 0.35
x = np.arange(len(df["parameters"].unique()))

mean = df["mean"]
rect1 = ax.bar(x, mean, width, label=f"Execution time (ms)")

ax.set_xticks(x, df["parameters"].unique())
ax.set_xlabel("Image")
ax.set_ylabel("Time (ms)")
ax.set_title(f"Mean time for each image segmentation [{num_clusters} clusters] \n[50 runs per task]")
ax.legend()

ax.bar_label(rect1, padding=4)

fig.tight_layout()

plt.savefig("results_segmentation.eps")
