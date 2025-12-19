import os
import time
import csv
from utils import load_n_instances, check_feasibility_n
from beamsearch import beam_search_glcs, imbs_glcs

BASE_INST_DIR = os.path.join("instance_i_generator", "instance")
SIGMAS = ["sigma-2", "sigma-4"]
RESULTS_DIR = "results"
CSV_FILE = os.path.join(RESULTS_DIR, "new_combined_beamsearch+imbs_results.csv")

os.makedirs(RESULTS_DIR, exist_ok=True)


def main():
    write_header = not os.path.exists(CSV_FILE)
    with open(CSV_FILE, "a", newline="") as csvfile:
        fieldnames = [
            "sigma", "filename", "beam_width", "func", "heuristic",
            "length", "time_s", "feasible"
        ]
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        if write_header:
            writer.writeheader()

        for sigma in SIGMAS:
            inst_dir = os.path.join(BASE_INST_DIR, sigma)
            for fname in sorted(os.listdir(inst_dir)):
                if not fname.endswith(".txt"):
                    continue

                file_in = os.path.join(inst_dir, fname)
                sequences, gaps = load_n_instances(file_in)

                for h in ["h2", "h5", "h8"]:
                    for beam_width in [1, 10, 50, 100, 250, 500, 1000, 2500, 10000]:
                        for func in [beam_search_glcs, imbs_glcs]:
                            full_heuristic = f"{h}"
                            if func == beam_search_glcs:
                                start = time.time()
                                best_seq, steps = func(
                                    sequences,
                                    gaps,
                                    beam_width=beam_width,
                                    heuristic=h,
                                )
                                elapsed = time.time() - start
                            elif func == imbs_glcs:
                                if beam_width > 2500:
                                    continue
                                start = time.time()
                                best_seq, steps = func(
                                    sequences,
                                    gaps,
                                    beam_width=beam_width,
                                    heuristic=h,
                                    number_of_roots=10,
                                    imbs_iters=10 #if beam_width >= 100 else 50
                                )
                                elapsed = time.time() - start
                            feasible = check_feasibility_n(steps, gaps, sequences)

                            writer.writerow({
                                "sigma": sigma,
                                "filename": fname,
                                "beam_width": beam_width,
                                "heuristic": full_heuristic,
                                "func": func.__name__,
                                "length": len(best_seq),
                                "time_s": f"{elapsed:.4f}",
                                "feasible": feasible,
                            })
                            print(f"Recorded: {sigma}/{fname} - {h} - {func.__name__} - {beam_width} done")

                        

if __name__ == "__main__":
    main()
