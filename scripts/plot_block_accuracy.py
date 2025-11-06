#!/usr/bin/env python
import argparse
import csv
from collections import OrderedDict
from pathlib import Path

import matplotlib.pyplot as plt


def load_block_accuracy(csv_path: Path):
    blocks = OrderedDict()
    with csv_path.open() as stream:
        reader = csv.DictReader(stream)
        for row in reader:
            key = (row["phase"], int(row["block"]))
            data = blocks.setdefault(key, {"total": 0, "correct": 0})
            data["total"] += 1
            data["correct"] += int(row["correct"])
    return [
        {
            "phase": phase,
            "block": block,
            "accuracy": (data["correct"] / data["total"]) if data["total"] else 0.0,
        }
        for (phase, block), data in blocks.items()
    ]


def plot_block_accuracy(blocks, title: str, output_path: Path):
    if not blocks:
        raise ValueError("No block data found for plotting.")

    x = list(range(1, len(blocks) + 1))
    y = [block["accuracy"] for block in blocks]
    labels = [f"{block['phase'][0].upper()}{block['block']}" for block in blocks]

    phase_boundaries = []
    prev_phase = blocks[0]["phase"]
    for idx, block in enumerate(blocks, start=1):
        if block["phase"] != prev_phase:
            phase_boundaries.append(idx - 0.5)
            prev_phase = block["phase"]

    fig, ax = plt.subplots(figsize=(10, 4))
    ax.plot(x, y, marker="o", linestyle="-", color="tab:blue")
    ax.set_xlabel("Block")
    ax.set_ylabel("Accuracy")
    ax.set_ylim(0.0, 1.05)
    ax.set_xticks(x, labels)
    ax.grid(True, axis="y", alpha=0.3)
    ax.set_title(title)

    for boundary in phase_boundaries:
        ax.axvline(boundary, color="gray", linestyle="--", alpha=0.4)

    text_y = ax.get_ylim()[1] - 0.05
    phase_start = 0.5
    last_phase = blocks[0]["phase"]
    for boundary in phase_boundaries + [len(blocks) + 0.5]:

        ax.text(
            (phase_start + boundary) / 2.0,
            text_y,
            last_phase.capitalize(),
            ha="center",
            va="top",
            fontsize=10,
            color="dimgray",
        )
        phase_start = boundary
        if boundary < len(blocks) + 0.5:
            idx = int(boundary + 0.5) - 1
            last_phase = blocks[idx]["phase"]

    fig.tight_layout()
    fig.savefig(output_path, dpi=200)
    plt.close(fig)


def parse_args():
    parser = argparse.ArgumentParser(description="Plot per-block accuracy for an experiment CSV.")
    parser.add_argument("--csv", type=Path, required=True, help="CSV file produced by MSC experiment export.")
    parser.add_argument("--output", type=Path, required=True, help="Path for the generated PNG.")
    parser.add_argument("--title", type=str, default="Experiment Block Accuracy", help="Plot title.")
    return parser.parse_args()


def main():
    args = parse_args()
    blocks = load_block_accuracy(args.csv)
    plot_block_accuracy(blocks, args.title, args.output)


if __name__ == "__main__":
    main()
