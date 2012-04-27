#include <elliptics/srw.hpp>

static void kill_all_fds(const char *log)
{
	int fd;

	for (int i = 0; i < 1024 * 1024; ++i) {
		close(i);
	}

	fd = open("/dev/null", O_RDWR);
	if (fd < 0) {
		fd = -errno;
		fprintf(stderr, "Can not open /dev/null: %d\n", fd);
		exit(fd);
	}

	dup2(fd, STDIN_FILENO);

	fd = open(log, O_RDWR);
	if (fd < 0) {
		fd = -errno;
		fprintf(stderr, "Can not open '%s': %d\n", log, fd);
		exit(fd);
	}

	dup2(fd, STDERR_FILENO);
	dup2(fd, STDOUT_FILENO);
}

int main(int argc, char *argv[])
{
	int ch, type = -1;
	std::string log("/dev/stdout"), pipe("/tmp/test-pipe"), init;

	while ((ch = getopt(argc, argv, "i:l:p:t:")) != -1) {
		switch (ch) {
			case 'i':
				init.assign(optarg);
				break;
			case 'l':
				log.assign(optarg);
				break;
			case 'p':
				pipe.assign(optarg);
				break;
			case 't':
				type = atoi(optarg);
				break;
			default:
				exit(-1);
		}
	}

	kill_all_fds(log.c_str());

	try {
		switch (type) {
			case SRW_TYPE_PYTHON: {
				ioremap::srw::worker<ioremap::srw::python> w(log, pipe, init);
				w.process();
				break;
			}
			default:
				exit(-1);
		}
	} catch (const std::exception &e) {
		std::ofstream l(log.c_str(), std::ios::app);
		l << getpid() << ": worker exception: " << e.what() << std::endl;
	}

	return 0;
}