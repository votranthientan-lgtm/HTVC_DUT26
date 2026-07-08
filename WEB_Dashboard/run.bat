@echo off
REM HTVC Backend Startup Script

echo ========================================
echo  HTVC - Tủ Rau Thông Minh Backend
echo ========================================
echo.

REM Check if virtual environment exists
if not exist "venv" (
    echo Creating virtual environment...
    python -m venv venv
)

REM Activate virtual environment
echo Activating virtual environment...
call venv\Scripts\activate.bat

REM Install dependencies
echo.
echo Installing dependencies...
pip install -r requirements.txt

REM Run the application
echo.
echo Starting Flask server...
echo Server will be available at http://localhost:5000
echo Press Ctrl+C to stop
echo.
python app.py

pause
