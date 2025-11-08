ARG BOOST_VERSION="1.89.0"
ARG BOOST_VARIANT="release"
ARG LINK="static"

FROM ghcr.io/zen0x7/compiler:${BOOST_VERSION}-${BOOST_VARIANT}-${LINK} AS builder
ARG BOOST_VARIANT
ARG LINK
ARG BOOST_VERSION

WORKDIR /srv/engine

RUN echo $LINK;
RUN echo $BOOST_VARIANT;
RUN echo $BOOST_VERSION;


COPY . .
RUN sh scripts/build.sh ${BOOST_VARIANT} ${LINK}

FROM alpine:latest AS runtime
WORKDIR /srv
COPY --from=builder /srv/build/server /usr/local/bin/server

ENTRYPOINT ["/usr/local/bin/server"]