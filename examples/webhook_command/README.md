# Webhook Command Example

Add a command to `sdcard/apps/webhook_launcher/commands.json`:

```json
{
  "name": "Start Backup",
  "method": "POST",
  "url": "https://example.com/webhook/start-backup",
  "confirm": true,
  "headers": {
    "Content-Type": "application/json",
    "Authorization": "Bearer REPLACE_WITH_TOKEN"
  },
  "body": {
    "source": "cardputer",
    "job": "backup"
  }
}
```

Use low-privilege webhook tokens and avoid placing real secrets in checked-in files.
