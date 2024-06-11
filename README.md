# Replication package for the paper "On the Use of Multi-valued Decision Diagrams to Count Valid Configurations of Feature Models"

## Package structure

* `benchmarks`:
	* `industrial`: set of industrial feature models
	* `synthetic`: set of synthetic feature models
* `benchmarks_converter`: Java program used to translate the SPLOT benchmarks into the FeatureIDE format. These models are used to reply to RQ1, RQ2, and RQ3, and stored in the `benchmarks/synthetic` folder
* `experiments_results`: evaluation scripts and results obtained with our experiments. These data are used to reply to all RQs in our paper
	* `images`: the folder contains the images produced by evaluation scripts and used in the paper
	* `BenchmarksAnalysis_RQ1.ipynb`: jupyter notebook used to answer RQ1
	* `BenchmarksAnalysis_RQ2.ipynb`: jupyter notebook used to answer RQ2
	* `BenchmarksAnalysis_RQ3.ipynb`: jupyter notebook used to answer RQ3
	* `BenchmarksAnalysis_RQ4.ipynb`: jupyter notebook used to answer RQ4
	* `experiments.csv`: it contains the results of the experiments (used in RQ1, RQ2, and RQ3) on synthetic benchmarks
	* `FMAnalysis.ipynb`: jupyter notebook used to extract the percentage of AND, OR and ALT groups in the `industrial` benchmarks
* `fm_counter_bdd`: the folder contains the java program used to compute the number of valid configurations using BDDs
	* `dist`: the folder contains the executable
		* `Counter.jar`: executable jar
		* `CountWithBDDs.sh`: bash script executing `Counter.jar` over all the `industrial` benchmarks
	* `src`: the folder contains the Java source code of the `fm_counter_bdd` project
* `fm_counter_mdd`: the folder contains the C++ program used to compute the number of valid configurations using MDDs, i.e., the approach we present in the paper
	* `dist`: the folder contains the executable
		* `FMBuilderExperimenter`: executable, compiled for Ubuntu
		* `run_experiments.sh`: bash script executing `FMBuilderExperimenter` over all the `synthetic` benchmarks
	* `src`: the folder contains the C++ source code of the `fm_counter_mdd` project