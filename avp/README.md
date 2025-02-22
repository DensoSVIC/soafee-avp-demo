# Lingua Franca implementation of the APV Demo

This is a port of Autoware to LF using the federated runtime in C.

The demo in this repository is not yet configured for turnkey execution on a standlone computer. It is currently built and provisioned by [DensoSVIC/soafee-avp-blueprint](https://github.com/DensoSVIC/soafee-avp-blueprint).

There are hints below on how to get started running the demo on a standalone machine. We recommend reviewing the blueprint to see the steps it takes to build and run all dependencies.

## Standalone Execution

### Build Lingua Franca

```shell
lfc src/Autoware.lf
```

### Build Docker

```shell
cd fed-gen/Autoware/src-gen
docker compose build
```
