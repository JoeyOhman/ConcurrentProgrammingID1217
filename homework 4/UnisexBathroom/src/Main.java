public class Main {

    public static final int MAX_WORK_TIME = 5000, MAX_BATHROOM_TIME = 1000;

    private static final int amountOfMen = 3, amountOfWomen = 3;

    public static void main(String[] args) {
        Bathroom bathroom = new Bathroom();
        Man[] men = new Man[amountOfMen];
        Woman[] women = new Woman[amountOfWomen];

        for (int i = 0; i < amountOfMen; i++)
            men[i] = new Man(bathroom);

        for (int i = 0; i < amountOfWomen; i++)
            women[i] = new Woman(bathroom);

        for (int i = 0; i < amountOfMen; i++)
            men[i].start();

        for (int i = 0; i < amountOfWomen; i++)
            women[i].start();
    }

}
