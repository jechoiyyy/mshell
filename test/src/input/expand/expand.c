/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jechoi <jechoi@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/03 16:11:42 by dsagong           #+#    #+#             */
/*   Updated: 2025/09/15 16:43:11 by jechoi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "expand.h"
#include "input.h"
#include <stdio.h>
#include "libft.h"
#include "state.h"
#include "global.h"

//확장로직
char	*expand_process(char *value, t_envp *envp_lst)
{
	t_exp	exp_data;

	if (!set_exp_data(&exp_data))
		return (NULL);
	while (value[exp_data.idx])
	{
		if (is_valid_expand(&exp_data, value))
		{
			if (!exp_env_process(&exp_data, value, envp_lst))
				return (NULL);
			continue ;
		}
		if (is_quote(value[exp_data.idx]))
			exp_data.state = state_machine(value[exp_data.idx], exp_data.state);
		if (!exp_normal_process(&exp_data, value))
			return (NULL);
		exp_data.idx++;
	}
	return (exp_data.result);
}

int	has_whitespace(const char *s)
{
	t_quote_state	state;
	size_t			i;

	if (!s)
		return (0);
	state = STATE_GENERAL;
	i = 0;
	while (s[i])
	{
		if (state == STATE_GENERAL && is_space((unsigned char)s[i]))
			return (1);
		if (s[i] == '\'' || s[i] == '"')
			state = state_machine(s[i], state);
		i++;
	}
	return (0);
}

//quote 일반 상태일때 whitespcae 존재시 파일문법 오류
static int	handle_word(t_token *curr, t_envp *envp_lst, int do_check_filename)
{
	char	*orig_value;
	char	*expanded;

	orig_value = curr->value;
	expanded = expand_process(curr->value, envp_lst);
	if (!expanded)
		return (0);
	if (do_check_filename)
	{
		if (expanded[0] == '\0' || has_whitespace(expanded))
		{
			curr->type = T_WRONG_FILNAME;
			free(expanded);
			return (1);
		}
		else
			curr->type = T_CORRECT_FILNAME;
	}
	free(orig_value);
	curr->value = expanded;
	return (1);
}

//변수 확장 및 filename 타입 세팅
//파일이름 문법 오류시 에러메세지 파이프 별로 하나씩 출력
//잘못된 파일인 경우 token type으로 명시
static int	handle_expand_word(t_token *curr, t_envp *envp_lst,
						int *do_check_filename, int *skip_expand)
{
	if (!*skip_expand)
	{
		if (!handle_word(curr, envp_lst, *do_check_filename))
			return (0);
	}
	*skip_expand = 0;
	*do_check_filename = 0;
	return (1);
}

static void	update_flags(t_token *curr, int *do_check_filename,
						int *skip_expand)
{
	if (curr->type == T_HEREDOC)
	{
		*skip_expand = 1;
		*do_check_filename = 0;
	}
	else if (curr->type == T_REDIR_IN || curr->type == T_REDIR_OUT
		|| curr->type == T_APPEND)
		*do_check_filename = 1;
}

int	expand_token(t_token *token_lst, t_envp *envp_lst)
{
	t_token	*curr;
	int		do_check_filename;
	int		skip_expand;

	curr = token_lst;
	do_check_filename = 0;
	skip_expand = 0;
	while (curr && curr->type != T_END)
	{
		if (curr->type == T_WORD)
		{
			if (!handle_expand_word(curr, envp_lst,
					&do_check_filename, &skip_expand))
				return (0);
		}
		else
			update_flags(curr, &do_check_filename, &skip_expand);
		curr = curr->next;
	}
	return (1);
}