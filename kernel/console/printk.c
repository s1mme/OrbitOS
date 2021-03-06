#include <console.h>
#include <system.h>
#include <string.h>
#include <vsprintf.h>
#include <serial.h>
#ifdef CONFIG_MULTIQUAD
#define LOG_BUF_LEN	(65536)
#elif defined(CONFIG_ARCH_S390)
#define LOG_BUF_LEN	(131072)
#elif defined(CONFIG_SMP)
#define LOG_BUF_LEN	(32768)
#else	
#define LOG_BUF_LEN	(16384)			/* This must be a power of two */
#endif

#define LOG_BUF_MASK	(LOG_BUF_LEN-1)

#ifndef arch_consoles_callable
#define arch_consoles_callable() (1)
#endif

/* printk's without a loglevel use this.. */
#define DEFAULT_MESSAGE_LOGLEVEL 4 /* KERN_WARNING */

/* We show everything that is MORE important than this.. */
#define MINIMUM_CONSOLE_LOGLEVEL 1 /* Minimum loglevel we let people use */
#define DEFAULT_CONSOLE_LOGLEVEL 7 /* anything MORE serious than KERN_DEBUG */

//DECLARE_WAIT_QUEUE_HEAD(log_wait);

int console_printk[4] = {
	DEFAULT_CONSOLE_LOGLEVEL,	/* console_loglevel */
	DEFAULT_MESSAGE_LOGLEVEL,	/* default_message_loglevel */
	MINIMUM_CONSOLE_LOGLEVEL,	/* minimum_console_loglevel */
	DEFAULT_CONSOLE_LOGLEVEL,	/* default_console_loglevel */
};

int oops_in_progress;

struct console *console_drivers;


static char log_buf[LOG_BUF_LEN];
#define LOG_BUF(idx) (log_buf[(idx) & LOG_BUF_MASK])

/*
 * The indices into log_buf are not constrained to LOG_BUF_LEN - they
 * must be masked before subscripting
 */
static unsigned long log_start;			/* Index into log_buf: next char to be read by syslog() */
static unsigned long con_start;			/* Index into log_buf: next char to be sent to consoles */
static unsigned long log_end;			/* Index into log_buf: most-recently-written-char + 1 */
static unsigned long logged_chars;		/* Number of chars produced since last read+clear operation */

struct console_cmdline console_cmdline[MAX_CMDLINECONSOLES];
static int preferred_console = -1;

/* Flag: console code may call schedule() */
static int console_may_schedule;

/*
 *	Setup a list of consoles. Called from init/main.c
 */
 int console_setup(char *str)
{
	struct console_cmdline *c;
	char name[sizeof(c->name)];
	char *s, *options;
	int i, idx;

	/*
	 *	Decode str into name, index, options.
	 */
	if (str[0] >= '0' && str[0] <= '9') {
		strcpy(name, "ttyS");
		strncpy(name + 4, str, sizeof(name) - 5);
	} else
		strncpy(name, str, sizeof(name) - 1);
	name[sizeof(name) - 1] = 0;
	if ((options = strchr(str, ',')) != NULL)
		*(options++) = 0;
#ifdef __sparc__
	if (!strcmp(str, "ttya"))
		strcpy(name, "ttyS0");
	if (!strcmp(str, "ttyb"))
		strcpy(name, "ttyS1");
#endif
	for(s = name; *s; s++)
		if (*s >= '0' && *s <= '9')
			break;
	idx = simple_strtoul(s, NULL, 10);
	*s = 0;

	/*
	 *	See if this tty is not yet registered, and
	 *	if we have a slot free.
	 */
	for(i = 0; i < MAX_CMDLINECONSOLES && console_cmdline[i].name[0]; i++)
		if (strcmp(console_cmdline[i].name, name) == 0 &&
			  console_cmdline[i].index == idx) {
				preferred_console = i;
				return 1;
		}
	if (i == MAX_CMDLINECONSOLES)
		return 1;
	preferred_console = i;
	c = &console_cmdline[i];
	memcpy(c->name, name, sizeof(c->name));
	c->options = options;
	c->index = idx;
	return 1;
}


/*
 * Call the console drivers on a range of log_buf
 */
static void __call_console_drivers(unsigned long start, unsigned long end)
{
	struct console *con;

	for (con = console_drivers; con; con = con->next) {
		if ((con->flags & CON_ENABLED) && con->write)
			con->write(con, &LOG_BUF(start), end - start);
	}
}

/*
 * Write out chars from start to end - 1 inclusive
 */
static void _call_console_drivers(unsigned long start, unsigned long end, int msg_log_level)
{
	if (msg_log_level < console_loglevel && console_drivers && start != end) {
		if ((start & LOG_BUF_MASK) > (end & LOG_BUF_MASK)) {
			/* wrapped write */
			__call_console_drivers(start & LOG_BUF_MASK, LOG_BUF_LEN);
			__call_console_drivers(0, end & LOG_BUF_MASK);
		} else {
			__call_console_drivers(start, end);
		}
	}
}

/*
 * Call the console drivers, asking them to write out
 * log_buf[start] to log_buf[end - 1].
 * The console_sem must be held.
 */
static void call_console_drivers(unsigned long start, unsigned long end)
{
	unsigned long cur_index, start_print;
	static int msg_level = -1;

	cur_index = start;
	start_print = start;
	while (cur_index != end) {
		if (	msg_level < 0 &&
			((end - cur_index) > 2) &&
			LOG_BUF(cur_index + 0) == '<' &&
			LOG_BUF(cur_index + 1) >= '0' &&
			LOG_BUF(cur_index + 1) <= '7' &&
			LOG_BUF(cur_index + 2) == '>')
		{
			msg_level = LOG_BUF(cur_index + 1) - '0';
			cur_index += 3;
			start_print = cur_index;
		}
		while (cur_index != end) {
		//	char c = LOG_BUF(cur_index);
			cur_index++;

				if (msg_level < 0) {
					/*
					 * printk() has already given us loglevel tags in
					 * the buffer.  This code is here in case the
					 * log buffer has wrapped right round and scribbled
					 * on those tags
					 */
					msg_level = default_message_loglevel;
				}
				_call_console_drivers(start_print, cur_index, msg_level);
				msg_level = -1;
				start_print = cur_index;
				break;
			
		}
	}
	_call_console_drivers(start_print, end, msg_level);
}

static void emit_log_char(char c)
{
	LOG_BUF(log_end) = c;
	log_end++;
	if (log_end - log_start > LOG_BUF_LEN)
		log_start = log_end - LOG_BUF_LEN;
	if (log_end - con_start > LOG_BUF_LEN)
		con_start = log_end - LOG_BUF_LEN;
	if (logged_chars < LOG_BUF_LEN)
		logged_chars++;
}

/*
 * This is printk.  It can be called from any context.  We want it to work.
 * 
 * We try to grab the console_sem.  If we succeed, it's easy - we log the output and
 * call the console drivers.  If we fail to get the semaphore we place the output
 * into the log buffer and return.  The current holder of the console_sem will
 * notice the new output in release_console_sem() and will send it to the
 * consoles before releasing the semaphore.
 *
 * One effect of this deferred printing is that code which calls printk() and
 * then changes console_loglevel may break. This is because console_loglevel
 * is inspected when the actual printing occurs.
 */
 int serial_printing = 1;
 int printk(const char *fmt, ...)
{
	va_list args;
	//unsigned long flags;
	int printed_len;
	char *p;
	static char printk_buf[81920];
	static int log_level_unknown = 1;

	va_start(args, fmt);
	printed_len = vsnprintf(printk_buf, sizeof(printk_buf), fmt, args);
	va_end(args);


if(serial_printing)
{
	if (printk_buf[strlen(printk_buf)] == '\n') {

		serial_string(0x3F8, printk_buf);
		char buf2[1024];
	
		serial_string(0x3F8, buf2);
	} else {
		serial_string(0x3F8 , printk_buf);
	}
}
	/*
	 * Copy the output into log_buf.  If the caller didn't provide
	 * appropriate log level tags, we insert them here
	 */
	for (p = printk_buf; *p; p++) { 
		if (log_level_unknown) {
			if (p[0] != '<' || p[1] < '0' || p[1] > '7' || p[2] != '>') {
				emit_log_char('<');
				emit_log_char(default_message_loglevel + '0');
				emit_log_char('>');
			}
			log_level_unknown = 0;
		}
		emit_log_char(*p); //<--------------- denna sparar texten som sedan printas ut med release_console_sem()
		if (*p == '\n')
			log_level_unknown = 1;
	}

		console_may_schedule = 0;
		release_console_sem();

	return printed_len;
}

void release_console_sem(void)
{
//	unsigned long flags;
	unsigned long _con_start, _log_end;
	unsigned long must_wake_klogd = 0;

	for ( ; ; ) {
		must_wake_klogd |= log_start - log_end;
		if (con_start == log_end)
			break;			/* Nothing to print */
		_con_start = con_start;
		_log_end = log_end;
		con_start = log_end;		/* Flush */

		call_console_drivers(_con_start, _log_end); //<--------------------------
	}
	console_may_schedule = 0;
}


void console_print(const char *s)
{
	printk(KERN_EMERG "%s", s);
}

/*
 * The console driver calls this routine during kernel initialization
 * to register the console printing procedure with printk() and to
 * print any messages that were printed by the kernel before the
 * console driver was initialized.
 */
void register_console(struct console * console)
{
	int     i;
//	unsigned long flags;

	/*
	 *	See if we want to use this console driver. If we
	 *	didn't select a console we take the first one
	 *	that registers here.
	 */
	if (preferred_console < 0) {
		if (console->index < 0)
			console->index = 0;
		if (console->setup == NULL ||
		    console->setup(console, NULL) == 0) {
			console->flags |= CON_ENABLED | CON_CONSDEV;
			preferred_console = 0;
		}
	}

	/*
	 *	See if this console matches one we selected on
	 *	the command line.
	 */
	for(i = 0; i < MAX_CMDLINECONSOLES && console_cmdline[i].name[0]; i++) {
		if (strcmp(console_cmdline[i].name, console->name) != 0)
			continue;
		if (console->index >= 0 &&
		    console->index != console_cmdline[i].index)
			continue;
		if (console->index < 0)
			console->index = console_cmdline[i].index;
		if (console->setup &&
		    console->setup(console, console_cmdline[i].options) != 0)
			break;
		console->flags |= CON_ENABLED;
		console->index = console_cmdline[i].index;
		if (i == preferred_console)
			console->flags |= CON_CONSDEV;
		break;
	}

	if (!(console->flags & CON_ENABLED))
		return;

	if ((console->flags & CON_CONSDEV) || console_drivers == NULL) {
		console->next = console_drivers;
		console_drivers = console;
	} else {
		console->next = console_drivers->next;
		console_drivers->next = console;
	}
	if (console->flags & CON_PRINTBUFFER) {

		con_start = log_start;

	}
	release_console_sem();
}


int unregister_console(struct console * console)
{
        struct console *a,*b;
	int res = 1;

	if (console_drivers == console) {
		console_drivers=console->next;
		res = 0;
	} else {
		for (a=console_drivers->next, b=console_drivers ;
		     a; b=a, a=b->next) {
			if (a == console) {
				b->next = a->next;
				res = 0;
				break;
			}  
		}
	}

	return res;
}


