FROM openadkit

ENV DEBIAN_FRONTEND=noninteractive
ENV DISPLAY=localhost:0.0
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1

ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

USER root

EXPOSE 9090

RUN apt-get update -q \
    && apt-get install -y -q --no-install-recommends \
        ros-foxy-lgsvl-bridge=0.2.1-1focal.20230527.053228 \
    && apt-get clean -y -q \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

RUN git clone --depth=1 --single-branch --branch 2021.3 https://github.com/lgsvl/PythonAPI.git
COPY lgsvl-bridge/app/requirements.txt /app/PythonAPI/requirements.txt

# version-locked requirements need to be updated in PythonAPI setup.py
# since later versions have breaking changes.
#
# read requirements.txt then add version specifiers to setup.py
WORKDIR /app/PythonAPI
RUN while IFS= read -r line; do \
        package=$(echo "$line" | cut -d= -f1 | xargs); \
        sed -i -E "s/(['\"]$package['\"][[:space:]]*)([,)]|$)/'$line'\2/" setup.py; \
    done < requirements.txt
RUN /app/PythonAPI/setup.py install --user

COPY lgsvl-bridge/app/entrypoint.sh /
COPY lgsvl-bridge/app/lgsvl-connecting-bridge.py /app/
COPY lgsvl-bridge/app/THIRD_PARTY_NOTICE.md /app
COPY lgsvl-bridge/app/third-party-licenses /app
COPY cyclonedds.xml /app/cyclonedds.xml

WORKDIR /app
ENTRYPOINT ["/entrypoint.sh"]
