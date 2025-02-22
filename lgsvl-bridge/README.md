# lgsvl-bridge

This image is built with a CycloneDDS configuration that lives in a parent context.
Note the paths in the build command.

```shell
(cd .. && docker build . --file lgsvl-bridge/Dockerfile --tag lgsvl-bridge)
```

The following ports should be mappped:

- 9090/TCP (lgsvl-bridge)
- 7410-7414 (CycloneDDS, varies by implementaton)

```shell
docker run -p 9090/tcp -p 7410-7414/udp lgsvl-bridge
```
