int initialize_nameserver();

/**
 * Register the task under the specified name.
 *
 *  0 Success
 * -1 Invalid NameServer Tid.
 * -2 NameServer Task Tid is not NameServer.
 */
int RegisterAs(char *name);

/**
 * Returns the tid of the named task.
 *
 * >=0 Tid
 *  -1 Invalid NameServer Tid.
 *  -2 NameServer Task Tid is not NameServer.
 *  -3 Invalid name
 */
int WhoIs(char *name);

