# share.file

A lightweight Windows application that allows you to instantly share files from your computer to any device on your local network by simply scanning a QR code.

## Download

**[📥 Download Latest Release (Windows x64)](https://github.com/sheuronazxe/sharefile/raw/refs/heads/main/release-x64/sharefile.exe)**

*No installation required - just download and run!*

## Features

- **Drag & Drop Interface**: Simply drag any file into the application window
- **Automatic QR Code Generation**: Instantly generates a QR code containing the download URL
- **Local Network Sharing**: Files are served over HTTP on your local network
- **Multi-file Support**: Navigate through multiple files with a single click
- **Customizable Port**: Choose your preferred HTTP port (default: 8080)
- **Automatic IP Detection**: Automatically detects your local IP address
- **Lightweight**: Small, fast, and no installation required

## How It Works

1. **Launch the Application**: Run `sharefile.exe`
2. **Set Your Port**: The default port is 8080, but you can change it if needed
3. **Drag & Drop**: Drag any file into the application window
4. **Scan QR Code**: Use your mobile device to scan the generated QR code
5. **Download**: Your device will automatically download the file

## Screenshots

*The application shows your local IP address and allows you to set a custom port. When you drag a file, it switches to QR code mode for easy scanning.*

## Requirements

- Windows 10 or later
- Local network connection (WiFi or Ethernet)
- Device with QR code scanner (smartphone, tablet, etc.)

## Security Note

This application creates a temporary HTTP server on your local network. The server is only accessible from devices on the same network and automatically stops when you close the application or press ESC.

## Usage Tips

- **Multiple Files**: If you drag multiple files at once, click anywhere on the QR code to cycle through them
- **Port Selection**: Use ports between 1024-65535. Avoid common ports like 80, 443, or 8080 if they're already in use
- **Network Issues**: If the QR code doesn't work, ensure both devices are on the same network and no firewall is blocking the connection
- **Exit QR Mode**: Press ESC to return to the main interface

## Technical Details

- Built with C++ and Win32 API
- Uses Mongoose embedded web server for HTTP functionality
- QR code generation via qrcodegen library
- Automatic local IP detection using Windows IP Helper API
- Supports file streaming for large files

## Building from Source

### Prerequisites
- Visual Studio 2019 or later
- Windows SDK

### Dependencies
- [Mongoose](https://github.com/cesanta/mongoose) - Embedded web server
- [QR Code Generator](https://github.com/nayuki/QR-Code-generator) - QR code generation library

### Build Steps
1. Clone the repository
2. Open the solution in Visual Studio
3. Build in Release mode
4. The executable will be generated in the output directory

## Author

Created by **sheuronazxe**

- GitHub: [@sheuronazxe](https://github.com/sheuronazxe)
- X (Twitter): [@sheuronazxe](https://x.com/sheuronazxe)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Troubleshooting

**QR Code doesn't scan properly:**
- Make sure both devices are on the same network
- Check if the port is being used by another application
- Verify your firewall settings

**"Port not allowed" error:**
- Use ports between 1024-65535
- Try a different port if the current one is in use

**Can't detect local IP:**
- Check your network connection
- Ensure you have an active network adapter

## Support

If you encounter any issues or have suggestions, please create an issue on GitHub.

---

**Made with ❤️ for easy file sharing**