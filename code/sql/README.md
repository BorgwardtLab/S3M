# Building the MIMIC Dataset 

In this section the reader is instructed how to reproduce our MIMIC dataset. This directory comprises a postgresql-pipeline to define sepsis cases and controls and to extract their vital time series from the MIMIC database following the recent SEPSIS-3 definition.

A large part of this code was inspired by this project: https://github.com/alistairewj/sepsis3-mimic. Some of their code reappears here modified, some basic scripts even untouched. A big thanks to their valuable contribution.
Therefore, if you use this code please cite both them (
Johnson, Alistair EW, David J. Stone, Leo A. Celi, and Tom J. Pollard. "The MIMIC Code Repository: enabling reproducibility in critical care research." Journal of the American Medical Informatics Association (2017): ocx084.
)
and us:
======> insert reference (of this publication)


## Getting Started

### Prerequisites

 a) Request Access to MIMIC (publicly available, however with permission procedure) https://mimic.physionet.org/gettingstarted/access/ 
 b) Download and install the MIMIC database according to documentation: https://mimic.physionet.org/gettingstarted/dbsetup/ 
unix/max: https://mimic.physionet.org/tutorials/install-mimic-locally-ubuntu/ 
windows: https://mimic.physionet.org/tutorials/install-mimic-locally-windows/ 

### Running the query

Once the requirements are fulfilled, run the query.sql from this folder using:

```
psql 'dbname=mimic user=your_user_name options=--search_path=mimiciii' -f query.sql 
```
