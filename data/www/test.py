import os

print("Hello, world")
if "HTTP_COOKIE" in os.environ:
	cookies = {c.split("=", 1)[0].strip() : c.split("=", 1)[1].strip() for c in os.environ["HTTP_COOKIE"].split(";")}
	if "username" in cookies:
		print("I recognize you,", cookies["username"])