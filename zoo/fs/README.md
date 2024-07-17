# FS - C++ File Access Library

This library provides an interface [iaccess](core/iaccess.h) that provides basic file operations.

There are currently two implementations:
1. [Local file system access](local/local_access.h)
2. [Sftp file system access](sftp/sftp_access.h)

Using the base class [iaccess](core/iaccess.h) allows for the implementation of generic algorithms. See [operations.cpp](core/operations.cpp) for an example of this.

## Quick start

All code samples imply:
```cpp
using namespace zoo::fs;
```

### Creating local access

```cpp
#include "zoo/fs/local/local_access.h"

void get_local_access()
{
	auto access = local::access{ std::make_shared<noop_interruptor>() };
	assert(!access.is_remote());
}

```

All access classes require a `std::shared_ptr<iinterruptor>` argument which is can be used to interrupt long during operations.
The `noop_interruptor` class is a dummy implementation of that interface.

### Creating sftp access

This is a bit more involved.
The user must supply an implementation of [sftp::issh_known_hosts](sftp/issh_knownhosts.h) and [sftp::issh_identity_factory](sftp/issh_identity_factory.h).
The former is responsible to verify and persist the host's public key hash. The latter for supplying SSH identities for public key authentication.
There is currently no default implementation that uses the files in `~/.ssh/`.


```cpp
#include "zoo/fs/sftp/sftp_access.h"

// User supplied known_hosts implementation.
class known_hosts : public sftp::issh_known_hosts
{
	result verify(const std::string& host, const std::string& pubkey_hash) override
	{
		// TO DO: Check if the public key hash matches the host name
		(void)host;
		(void)pubkey_hash;
		return result::KNOWN;
	}

	// The persist method is called in one of the following cases:
	// (1) verify returned result::UNKNOWN and the `allow_unknown_host_key` option is true.
	// (2) verify returned result::CHANGED and the `allow_changed_host_key` option is true.
	void persist(const std::string& host, const std::string& pubkey_hash) override
	{
		// TO DO: Persist the public key hash for the host
		(void)host;
		(void)pubkey_hash;
	}
};

constexpr auto myident_pkey = R"~(
-----BEGIN OPENSSH PRIVATE KEY-----
b3BlbnNzaC1rZXktdjEAAAAABG5vbmUAAAAEbm9uZQAAAAAAAAABAAAAMwAAAAtzc2gtZW
QyNTUxOQAAACCNqM0YbBCgvhmFT79BUZMm0CljBmG1U3UrJp5eJng1gwAAAKBWkyW9VpMl
vQAAAAtzc2gtZWQyNTUxOQAAACCNqM0YbBCgvhmFT79BUZMm0CljBmG1U3UrJp5eJng1gw
AAAEDgXGGE44dwbbapezowduNKrrCZ9Oqvc5igjdGLEYdlqY2ozRhsEKC+GYVPv0FRkybQ
KWMGYbVTdSsmnl4meDWDAAAAFnBhdHJpY2tyQGRzcy1wb3J0LTAwMzYBAgMEBQYH
-----END OPENSSH PRIVATE KEY-----
)~";

// User supplied SSH identity factory
class ssh_identity_factory : public sftp::issh_identity_factory
{
	std::vector<std::shared_ptr<sftp::ssh_identity>> create() override
	{
		// TO DO: Create a list of SSH identities.
		// These identities will be offered to the SSH server (if it supports public key authentication)
		// The name of the identities are only used for logging purposes.
		return { std::make_shared<sftp::ssh_identity>("my identity", myident_pkey) };
	}
};

void get_sftp_access()
{
	// The `host` and `user` members of `sftp::options` are required.
	// The `port` member is optional and defaults to 22.
	// The `password` member is optional. If supplied, it may be used for password authentication, if the SSH server allows it.
	// The identities supplied by the SSH identity factory will used if the SSH server allows public key authentication.
	// If the SSH identity factory returns an empty vector, then public key authentication will be skipped.
	// Generally, the authentication process will try the following methods, in order, and only if the SSH server allows them:
	// (1) None method
	// (2) Public key
	// (3) Password
	auto access = sftp::access{ sftp::options{ .host = "example.net", .port = 22, .user = "me", .password = "mysecret123%" },
		                        std::make_shared<known_hosts>(),
		                        std::make_shared<ssh_identity_factory>(),
		                        std::make_shared<noop_interruptor>() };
	assert(access.is_remote());
}
```

### Access operations

```cpp
#include "zoo/fs/core/direntry.h"
#include "zoo/fs/core/ifile.h"
#include "zoo/fs/core/iwatcher.h"

void access_operations(iaccess& a)
{
	// All iaccess methods will throw an exception if an error occurs,
	// e.g. no permission etc...

	// Get all directory entries of the current directory.
	const auto entries = a.ls(".");

	// Check the existence of a file (or dir, or ...) named "foo/bar";
	if (a.exists("foo/bar"))
	{
		std::cout << "foo/bar exists\n";
	}

	// Try to get stat info of the file "foo/bar/baz"
	// Returns an std::optional which will be null if the file does not exist.
	const auto baz = a.try_stat("foo/bar/baz");
	if (baz)
	{
		std::cout << baz.value().mode_string() << " foo/bar/baz\n";
	}
	else
	{
		std::cout << "foo/bar/baz does not exist\n";
	}

	// Get the stat info of the file "/bar"
	// Will throw if the file does not exist.
	const auto bar = a.stat("/bar");
	std::cout << bar.mode_string() << " /bar\n";

	// Note: The lstat variants work like stat but without dereferencing symlinks.

	// Remove a file
	a.remove("/some/file");

	// Create a directory, non-recursive
	a.mkdir("/tmp/foodir", false);

	// Create a directory recursively
	a.mkdir("/tmp/some/nested/path/foodir", true);

	// Rename/move a file
	a.rename("foo/bar", "/tmp/baz");

	// Open a file
	// The `ifile` interface provides the methods `read` and `write`
	auto       f1 = a.open("/foo/bar", O_RDONLY, 0);
	char       buf[64];
	const auto count_read = f1->read(buf, sizeof(buf));

	auto       f2            = a.open("/tmp/bar", O_WRONLY | O_TRUNC, 0644);
	const auto count_written = f2->write(buf, count_read);
}

// `source_access` and `dest_access` may be equal or different.
void copy_file(iaccess& source_access, const fspath& source, iaccess& dest_access, const fspath& dest)
{
	auto in  = source_access.open(source, O_RDONLY | O_BINARY, 0);
	auto out = dest_access.open(dest, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644);

	std::array<char, 65536u> buf{};

	for (;;)
	{
		auto count = in->read(buf.data(), buf.size());
		if (count == 0)
		{
			break;
		}
		else
		{
			auto ptr = buf.data();
			while (count)
			{
				const auto written = out->write(ptr, count);
				count -= written;
				ptr += written;
			}
		}
	}
}

void directory_watcher(iaccess& source_access, const fspath& source_dir, iaccess& dest_access, const fspath& dest_dir)
{
	// Watch the source directory for incoming files
	// Incoming means newly written or renamed-to files.
	auto watcher = source_access.create_watcher(source_dir);
	for (;;)
	{
		const auto entries = watcher->watch();
		for (const auto& entry : entries)
		{
			// If the entry is a regular file
			if (entry.attr.is_reg())
			{
				copy_file(source_access, source_dir / entry.name, dest_access, dest_dir / entry.name);
			}
		}
	}
}
```

## Motivation

I was working on a tool that monitors a local or remote directory for incoming files and, given some conditions, run a configurable workflow on these files.
The file system access needed abstraction to achieve this, so I decided to put this in a separate library.
