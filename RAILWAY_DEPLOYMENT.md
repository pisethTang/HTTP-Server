# 🚂 Deploying Titan HTTP Server to Railway

## Prerequisites
- GitHub account (Railway uses GitHub OAuth)
- This project pushed to a GitHub repository

## Step-by-Step Instructions

### 1. **Prepare Your GitHub Repository**
```bash
# Initialize git if you haven't
git init

# Add all files
git add .

# Commit
git commit -m "Prepare for Railway deployment"

# Create a repo on GitHub, then push
git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPO.git
git branch -M main
git push -u origin main
```

### 2. **Sign Up for Railway**
1. Go to [railway.app](https://railway.app)
2. Click **"Login"** (top right)
3. Choose **"Login with GitHub"**
4. Authorize Railway to access your GitHub

### 3. **Deploy Your Project**
1. Click **"New Project"**
2. Select **"Deploy from GitHub repo"**
3. Choose your `HTTP-Server` repository
4. Railway will auto-detect the Dockerfile and start building

### 4. **Configure the Deployment**
Once deployed:
1. Go to your project's **Settings** tab
2. Under **Networking**, click **"Generate Domain"**
3. You'll get a public URL like `https://your-app.up.railway.app`

### 5. **View Logs**
Click on the **Deployments** tab to see:
- Build logs
- Runtime logs (you should see "Server listening on port 8080...")

## 🎯 Testing Your Deployment

Once live, test these endpoints:
- `https://your-app.up.railway.app/` - Home page
- `https://your-app.up.railway.app/stats` - JSON stats
- `https://your-app.up.railway.app/dashboard` - Dashboard
- `https://your-app.up.railway.app/time` - CGI Python demo

## ⚠️ Important Notes

### Port Configuration
Railway provides a `PORT` environment variable, but your server currently hardcodes port 8080. Railway will proxy external traffic to your container's port 8080, so this works fine.

If you want to be more flexible:
```cpp
// In server.cpp, change:
int PORT = 8080;

// To read from environment:
const char* port_env = getenv("PORT");
int PORT = port_env ? atoi(port_env) : 8080;
```

### Stress Testing
**DO NOT run stress_test.py against your Railway deployment!** 
- Railway has rate limits
- Opening 2000+ connections will likely get you throttled or banned
- Only run stress tests locally

### File Uploads
The `/upload` endpoint saves files to the container's filesystem. These will be **lost on restart** because containers are ephemeral. For production:
- Use a database (PostgreSQL, MongoDB)
- Use cloud storage (S3, Railway Volumes)

### Free Tier Limits
Railway's free plan includes:
- $5 credit/month
- ~500 hours of runtime
- Good for demos and learning!

## 🔧 Troubleshooting

**Build fails?**
- Check the build logs in Railway dashboard
- Make sure Dockerfile is in the root directory

**Server not responding?**
- Check runtime logs for errors
- Ensure your server is actually listening (look for "Server listening..." in logs)

**Can't connect?**
- Make sure you generated a public domain in Settings → Networking
- Check that port 8080 is EXPOSE'd in Dockerfile (it is)

## 🚀 Next Steps

Once deployed, share your live URL!
Example: "Check out my HTTP server built with raw Linux syscalls: https://titan-server.up.railway.app"

Perfect for:
- Portfolio projects
- Demonstrating systems programming knowledge
- Showing off to recruiters 😎
