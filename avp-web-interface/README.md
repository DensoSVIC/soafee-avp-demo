# avp-web-interface

This image is built with a CycloneDDS configuration that lives in a parent context.
Note the paths in the build command.

```shell
(cd .. && docker build . --file avp-web-interface/Dockerfile --tag avp-web-interface)
```

The following ports should be mappped:

- 9090/TCP (lgsvl-bridge)
- 7410-7414 (CycloneDDS, varies by implementaton)

```shell
docker run -p 8000/tcp -p 9090/tcp avp-web-interface
```
