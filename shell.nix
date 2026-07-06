{
  pkgs ? import <nixpkgs> { },
  unstablePkgs,
  riscvPkgs,
}:
pkgs.mkShell {
  hardeningDisable = [
    "relro"
    "bindnow"
  ];

  packages = with pkgs; [
    unstablePkgs.gcc
    bear
    glibc_multi
    gtkwave
    iverilog
    screen
    xxd

    riscvPkgs.buildPackages.binutils
    riscvPkgs.buildPackages.gcc
    riscvPkgs.buildPackages.gcc.cc.lib
    riscvPkgs.newlib-nano
  ];
}
