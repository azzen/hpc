import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import glob
import os

input_images = glob.glob("../code/images/*.png")
input_type = ["1", "2"]
test_matrix = []

for i, image in enumerate(input_images):
    for j, type in enumerate(input_type):
        test_matrix.append(f"\"{image} out.png {type}\"")

command = f"""
hyperfine --runs 50 \
            -L param {",".join(test_matrix)} \
            --export-csv benchmark.csv \
            '../code/build/edge_detection {{param}}'
"""

if not os.path.exists("benchmark.csv"):
    os.system(command)
    print("Cleaning up...")
    os.system("rm out.png")
else:
    print("Benchmark already exists skipping...")

print("Generating graphs")

df = pd.read_csv("benchmark.csv")

# Strip prefix from parameters column
df["parameter_param"] = df["parameter_param"].str.replace("../code/images/", "")
df["parameter_param"] = df["parameter_param"].str.replace(" out.png", "")

# rename parametesr_param to parameters
df = df.rename(columns={"parameter_param": "parameters"})

# separate data where type is 1 and 2 and create two df
df["type"] = df["parameters"].str.extract(r"(\d)$")
df["parameters"] = df["parameters"].str.replace(" 1", "")
df["parameters"] = df["parameters"].str.replace(" 2", "")

# convert s to ms
df["mean"] = df["mean"] * 1000

# plot rect graph for each image with two bars for each type
fig, ax = plt.subplots()
width = 0.35
x = np.arange(len(df["parameters"].unique()))

type1_mean = df[df["type"] == "1"]["mean"]
type2_mean = df[df["type"] == "2"]["mean"]
rect1 = ax.bar(x - width/2, type1_mean, width, label="Type 1 (1D Array)")
rect2 = ax.bar(x + width/2, type2_mean, width, label="Type 2 (Linked List)")

ax.set_xticks(x, df["parameters"].unique())
ax.set_xlabel("Image")
ax.set_ylabel("Time (ms)")
ax.set_title("Mean time for each image processing by data structures\n[50 runs per task]")
ax.legend()

ax.bar_label(rect1, padding=4)
ax.bar_label(rect2, padding=4)

fig.tight_layout()

plt.savefig("results.eps")
