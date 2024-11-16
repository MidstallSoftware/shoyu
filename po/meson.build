i18n = import('i18n')

xgettext_args = [
  '--msgid-bugs-address="https://github.com/MidstallSoftware/shoyu/issues/"',
  '--add-comments',
  '--from-code=utf-8',
  '--flag=g_dngettext:2:pass-c-format',
  '--flag=g_strdup_printf:1:c-format',
  '--flag=g_string_printf:2:c-format',
  '--flag=g_string_append_printf:2:c-format',
  '--flag=g_error_new:3:c-format',
  '--flag=g_set_error:4:c-format',
  '--flag=g_markup_printf_escaped:1:c-format',
  '--flag=g_log:3:c-format',
  '--flag=g_warning:1:c-format',
  '--flag=g_error:1:c-format',
  '--flag=g_critical:1:c-format',
  '--flag=g_message:1:c-format',
  '--flag=g_debug:1:c-format',
  '--flag=g_print:1:c-format',
  '--flag=g_printerr:1:c-format',
  '--flag=g_printf:1:c-format',
  '--flag=g_fprintf:2:c-format',
  '--flag=g_sprintf:2:c-format',
  '--flag=g_snprintf:3:c-format',
  '--flag=g_scanner_error:2:c-format',
  '--flag=g_scanner_warn:2:c-format',
]

i18n.gettext('shoyu', args: xgettext_args)
