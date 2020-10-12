{ stdenv, cmake, pkg-config, gtest, cpr, curlFull, botan2 }:
stdenv.mkDerivation {
  name = "toytorrent";
  src = ./.;
  nativeBuildInputs = [ cmake curlFull.dev pkg-config ];
  buildInputs = [
    cpr
    botan2
    gtest
  ]; # gtest so integration that integration tests can be run in arion-compose network
  doCheck =
    false; # Integration tests are run in the arion-compose network, not here
  cmakeFlags = [ "-DBUILD_TESTING=ON" "-DINSTALL_TESTS=ON" ];
  checkInputs = [ gtest ];
}
