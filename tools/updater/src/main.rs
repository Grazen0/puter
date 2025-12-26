use std::{fs, io::Write, time::Duration};

use clap::Parser;
use serialport::{DataBits, Parity, StopBits};

#[derive(Parser)]
#[command(version, about, long_about = None)]
struct Args {
    file: String,

    /// Path to the serial port.
    #[arg(short, long, default_value = "/dev/ttyUSB0")]
    port: String,

    /// Baud rate to use for communication
    #[arg(short, long, default_value = "9600")]
    baud_rate: u32,

    /// Connection timeout in milliseconds
    #[arg(short, long, default_value = "5000")]
    timeout: u64,
}

fn main() -> anyhow::Result<()> {
    let args = Args::parse();

    let bin = fs::read(args.file)?;

    let mut port = serialport::new(args.port, args.baud_rate)
        .data_bits(DataBits::Eight)
        .parity(Parity::Even)
        .stop_bits(StopBits::One)
        .timeout(Duration::from_millis(args.timeout))
        .open()?;

    loop {
        let mut buf = [0];
        while port.bytes_to_read()? == 0 {}

        port.read_exact(&mut buf)?;

        print!("{}", buf[0] as char);
        std::io::stdout().flush()?;
    }
}
