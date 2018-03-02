import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Monitor
 */
public class Bathroom {

    private Lock lock = new ReentrantLock(true); // Mutex
    private Condition men = lock.newCondition(); // Condition men
    private Condition women = lock.newCondition(); // Condition women

    private int numberMen = 0, numberWomen = 0, delayedMen = 0, delayedWomen = 0;

    private long startTime;

    public Bathroom() {
        startTime = System.currentTimeMillis();
    }

    public void manEnter(long id) throws InterruptedException {

        lock.lock();
        // Wait if women inside or women in queue
        if (numberWomen > 0 || delayedWomen > 0) {
            delayedMen++;
            //while (numberWomen > 0 || delayedWomen > 0) {
            printQueue(true, id);
            men.await();
            //}
        }
        // Enter
        numberMen++;
        // Wake one, since all can't acquire lock at the same time anyway
        if (delayedMen > 0) {
            delayedMen--;
            men.signal();
        }

        printEnter(true, id);
        lock.unlock();

    }

    public void manExit(long id) {
        lock.lock();

        numberMen--;
        printLeave(true, id);
        // Pass the baton to delayed woman
        if (numberMen == 0 && delayedWomen > 0) {
            delayedWomen--;
            women.signal();
            logLine("Women's turn!\n");
        } else if(numberMen == 0)
            logLine("Bathroom empty!\n");

        lock.unlock();
    }

    public void womanEnter(long id) throws InterruptedException {
        lock.lock();
        // Wait if men inside or men in queue
        if (numberMen > 0 || delayedMen > 0) {
            delayedWomen++;
            printQueue(false, id);
            //while (numberMen > 0 || delayedMen > 0) {
            women.await();
            //}
        }
        // Enter
        numberWomen++;
        // Wake one, since all can't enter synchronized at the same time anyway
        if (delayedWomen > 0) {
            delayedWomen--;
            women.signal();
        }

        printEnter(false, id);
        lock.unlock();
    }

    public void womanExit(long id) {
        lock.lock();

        numberWomen--;
        printLeave(false, id);
        // Pass the baton to delayed man
        if (numberWomen == 0 && delayedMen > 0) {
            delayedMen--;
            men.signal();
            logLine("Men's turn!\n");
        } else if(numberWomen == 0)
            logLine("Bathroom empty!\n");

        lock.unlock();
    }

    private void printEnter(boolean man, long id) {
        if (man)
            logLine("Man #" + id + " entered bathroom!");
        else
            logLine("Woman #" + id + " entered bathroom!");


        //System.out.println("\t\t***Men inside: " + numberMen + ", Men delayed: " + delayedMen + "***");
        //System.out.println("\t\t***Women inside: " + numberWomen + ", Women delayed: " + delayedWomen + "***");
    }

    private void printQueue(boolean man, long id) {
        if (man)
            logLine("Man #" + id + " queued!");
        else
            logLine("Woman #" + id + " queued!");
    }

    private void printLeave(boolean man, long id) {
        if (man)
            logLine("Man #" + id + " left bathroom!");
        else
            logLine("Woman #" + id + " left bathroom!");
    }

    private void logLine(String line) {
        String timeStamp = createTimeStamp((System.currentTimeMillis() - startTime) / 1000);
        System.out.println(timeStamp + ": " + line);
    }

    private String createTimeStamp(long seconds) {
        long tens = seconds / 10;
        long ones = seconds % 10;
        return Long.toString(tens) + Long.toString(ones);
    }

}
