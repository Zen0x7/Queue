ARG BOOST_VERSION="1.89.0"
ARG BOOST_VARIANT="release"
ARG LINK="shared"

FROM ghcr.io/zen0x7/compiler:${BOOST_VERSION}-${LINK}-${BOOST_VARIANT}

WORKDIR /srv/engine

COPY . .
RUN sh scripts/build.sh ${BOOST_VARIANT} ${LINK}