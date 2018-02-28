public class Woman extends Thread {

    private Bathroom bathroom;

    public Woman(Bathroom bathroom) {
        this.bathroom = bathroom;
    }

    public void run() {
        while(true) {
            work();
            try {
                bathroom.womanEnter(getId());
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            useBathroom();
            bathroom.womanExit(getId());
        }
    }

    private void work() {
        try {
            sleep((int)(Math.random()*Main.MAX_WORK_TIME));
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private void useBathroom() {
        try {
            sleep((int)(Math.random()*Main.MAX_BATHROOM_TIME));
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

}
