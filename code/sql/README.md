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

1. MIMIC-3 Access
 a) Request Access to MIMIC (publicly available, however with permission procedure) https://mimic.physionet.org/gettingstarted/access/ 
 b) Download and install the MIMIC database according to documentation: https://mimic.physionet.org/gettingstarted/dbsetup/ 
unix/max: https://mimic.physionet.org/tutorials/install-mimic-locally-ubuntu/ 
windows: https://mimic.physionet.org/tutorials/install-mimic-locally-windows/ 

```
Give examples
```

### Installing

A step by step series of examples that tell you have to get a development env running

Say what the step will be

```
Give the example
```

And repeat

```
until finished
```

End with an example of getting some data out of the system or using it for a little demo

## Running the tests

Explain how to run the automated tests for this system

### Break down into end to end tests

Explain what these tests test and why

```
Give an example
```

### And coding style tests

Explain what these tests test and why

```
Give an example
```

## Deployment

Add additional notes about how to deploy this on a live system

## Built With

* [Dropwizard](http://www.dropwizard.io/1.0.2/docs/) - The web framework used
* [Maven](https://maven.apache.org/) - Dependency Management
* [ROME](https://rometools.github.io/rome/) - Used to generate RSS Feeds

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/your/project/tags). 

## Authors

* **Billie Thompson** - *Initial work* - [PurpleBooth](https://github.com/PurpleBooth)

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Hat tip to anyone who's code was used
* Inspiration
* etc
