ARG BOOST_VERSION="1.89.0"
ARG BOOST_VARIANT="release"
ARG LINK="static"

FROM ghcr.io/zen0x7/compiler:${BOOST_VERSION}-${LINK}-${BOOST_VARIANT} AS builder
WORKDIR /srv

COPY . .
RUN sh scripts/build.sh ${BOOST_VARIANT} ${LINK}

FROM alpine:latest AS runtime
WORKDIR /srv
COPY --from=builder /srv/build/server /usr/local/bin/server

ENTRYPOINT ["/usr/local/bin/server"]