FROM openadkit

ENV DEBIAN_FRONTEND=noninteractive
ENV DISPLAY=localhost:0.0
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1

ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

EXPOSE 8000
EXPOSE 9090

USER root

COPY avp-web-interface/entrypoint.sh /
COPY avp-web-interface/web/ /app/web/
COPY ./cyclonedds.xml /app/

WORKDIR /app
ENTRYPOINT ["/entrypoint.sh"]
