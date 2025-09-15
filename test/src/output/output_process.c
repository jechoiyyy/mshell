#include "output.h"

int	output_process(t_shell *shell, t_prompt *prompt)
{
	t_cmd	*cmd;

	cmd = parse_tokens(prompt);
	if (!cmd)
		return (FAILURE);
	if (execute_pipeline(cmd, shell) == FAILURE)
	{
		free_commands(cmd);
        return (FAILURE);
	}
	free_commands(cmd);
	return (g_exit_status);
}
